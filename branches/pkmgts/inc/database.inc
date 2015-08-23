<?php
include_once "inc/funcs.inc";

class Database {
  private $link;
  private $db;
  private $count = array("query"=>0, "error"=>0);
  public function __construct() {
    global $cfg;
    $this->link = @mysql_connect($cfg['mysql'][0], $cfg['mysql'][1], $cfg['mysql'][2]);
    if(!$this->link) {
      $cfg['mysqlfatal'] = mysql_error();
      return false;
    }
    $this->db = @mysql_select_db($cfg['mysql'][3], $this->link);
    if(!$this->db) {
      $cfg['mysqlfatal'] = mysql_error();
      return false;
    }

    // check .pkm files that they are the expected 292 bytes (smaller means bad encoded, and results in blue screens on the NDS)
    if(is_dir($cfg['distpkm'])) {
      $dir = $cfg['distpkm'];
      if($dh = opendir($dir)) {
        while(($file = readdir($dh)) !== false) {
          $info = pathinfo($file);
          if(strtolower($info['extension']) != "pkm")
            continue;
          $valid = 0;
          $prop = $dir."/".$file.".proper";
          if(is_array($cfg['validpkmpropersizes']) && count($cfg['validpkmpropersizes']) > 0) {
            foreach($cfg['validpkmpropersizes'] as $fsize)
              if(file_exists($prop) && !$valid && filesize($prop) == $fsize)
                $valid = 1;
            if($valid)
              $this->eventActivate($file);
            else
              $this->eventDeactivate($file);
          } elseif(file_exists($prop) && filesize($prop) != 292) // fallback sizecheck value
            $this->eventDeactivate($file);
        }
        closedir($dh);
      }
    }

    // establish "vars" (when db-table is fresh)
    $result = $this->query("select", "vars", array("WHERE k='%s'"=>"round"));
    if(!$this->queryOk($result) || count($result) == 0)
      $this->queryInsUpd("vars", array("k", "v"), array("round", "1"), array("k", "round")); // round = 1 (integer)
    $result1 = $this->query("select", "vars", array("WHERE k='%s'"=>"nextround"));
    if(!$this->queryOk($result1) || count($result1) == 0)
      $this->queryInsUpd("vars", array("k", "v"), array("nextround", strtotime("+{$cfg['roundtime']} minutes")), array("k", "nextround")); // nextRound = now + X minutes (timestamp)
    $result2 = $this->query("select", "vars", array("WHERE k='%s'"=>"votescleared"));
    if(!$this->queryOk($result2) || count($result2) == 0)
      $this->queryInsUpd("vars", array("k", "v"), array("votescleared", "0"), array("k", "votescleared")); // thisRoundVotesCleared = 0 (false)

    // stop here if server is offline, this way no new rounds are launched
    if(!$cfg['gtsonline'])
      return true;

    // cleanup and background work
    $this->bgCleanup();

    // validate integrity (fix if something is broken)
    if(!$this->validIntegrity()) {
      $this->newRound();
      $this->clearVotes();
    }

    return true;
  }
  public function ready() {
    return is_resource($this->link) && $this->db;
  }
  public function shutdown() {
    return mysql_close($this->link);
  }
  public function query($type, $table, $data=0, $fields=0, $sort=0, $limit=0) {
    if(!$this->link)
      return;
    $query = "";
    switch($type) {
      case "select":
        $query .= "SELECT ";
        if(is_array($fields) && count($fields) > 0) {
          foreach($fields as $field)
            $query .= "{$field}, ";
          $query = substr($query, 0, strlen($query)-2)." ";
        } else $query .= "* ";
        $query .= "FROM `{$table}` ";
        if(is_array($data) && count($data) > 0) {
          foreach($data as $k=>$v)
            if(is_numeric($k)) {
              $query .= "{$v} ";
            } else {
              $query .= sprintf("{$k} ", mysql_real_escape_string($v));
            }
        }
        if(is_array($sort) && count($sort) > 0) {
          $query .= "ORDER BY ";
          foreach($sort as $field=>$order)
            $query .= "`{$field}` {$order}, ";
          $query = substr($query, 0, strlen($query)-2)." ";
        }
        if(is_numeric($limit) && $limit > 0)
          $query .= "LIMIT {$limit};";
        break;

      case "update":
        $query .= "UPDATE `{$table}` SET ";
        if(is_array($data) && count($data) > 0) {
          foreach($data as $k=>$v)
            $query .= sprintf("`{$k}`='%s', ", mysql_real_escape_string($v));
          $query = substr($query, 0, strlen($query)-2)." ";
        }
        if(is_array($fields) && count($fields) > 0) {
          $query .= "WHERE ";
          foreach($fields as $k=>$v)
            $query .= sprintf("`{$k}`='%s' AND ", mysql_real_escape_string($v));
          $query = substr($query, 0, strlen($query)-5)." ";
        }
        break;

      case "insert":
        $query .= "INSERT INTO `{$table}` (";
        if(count($data) > 0 && count($data) == count($fields)) {
          foreach($data as $field)
            $query .= "`{$field}`, ";
          $query = substr($query, 0, strlen($query)-2);
          $query .= ") VALUES (";
          foreach($fields as $value)
            $query .= sprintf("'%s', ", mysql_real_escape_string($value));
          $query = substr($query, 0, strlen($query)-2);
          $query .= ")";
        }
        break;

      default:
        $query = $type;
        break;
    }
    $query = trim($query);
    #echo $query."\n";#die();
    $result = @mysql_query($query, $this->link);
    if($result) {
      $this->count['query']++;
      if($type == "select") {
        $rows = array();
        while($row = mysql_fetch_assoc($result))
          $rows[] = utf8($row);
        mysql_free_result($result);
        return $rows;
      } elseif(mysql_affected_rows($this->link) > 0)
        return true;
    } else $this->count['error']++;
    return false;
  }
  public function queryInsUpd($table, $fields, $values, $updIdent=0) {
    $result = $this->query("select", $table, array("WHERE `{$updIdent[0]}`='%s'"=>$updIdent[1]));
    if(!is_array($result) || count($result) == 0) {
      return $this->query("insert", $table, $fields, $values);
    } else {
      $update = array();
      foreach($fields as $k=>$v)
        $update[$v] = $values[$k];
      return $this->query("update", $table, $update, array("{$updIdent[0]}"=>$updIdent[1]));
    }
    return false;
  }
  public function queryOk($result) {
    if(is_array($result) || is_bool($result) && $result === true)
      return true;
    return false;
  }
  private function clearVotes() {
    $this->query("TRUNCATE TABLE `tmpvotes`", 1);
    $return = $this->queryInsUpd("vars", array("k", "v"), array("votescleared", "1"), array("k", "votescleared"));
    return $this->queryOk($return);
  }
  private function newRound() {
    global $cfg;
    $this->pickWinner();
    $round = $this->query("select", "vars", array("WHERE k='%s'"=>"round"));
    $round = @$round[0]['v'];
    $return1 = $this->queryInsUpd("vars", array("k", "v"), array("round", $round+1), array("k", "round"));
    $return2 = $this->queryInsUpd("vars", array("k", "v"), array("nextround", strtotime("+{$cfg['roundtime']} minutes")), array("k", "nextround"));
    $return3 = $this->queryInsUpd("vars", array("k", "v"), array("votescleared", "0"), array("k", "votescleared"));
    $this->clearVotes();
    return $this->queryOk($return1) && $this->queryOk($return2);
  }
  private function bgCleanup() {
    global $cfg;
    $result = $this->query("select", "vars", array("WHERE k='%s'"=>"round"));
    $result1 = $this->query("select", "vars", array("WHERE k='%s'"=>"nextround"));
    $result2 = $this->query("select", "vars", array("WHERE k='%s'"=>"votescleared"));
    if($result1[0]['v']-time() < 0)
      $this->newRound();
    if($result2[0]['v'] == "0")
      $this->clearVotes();
  }
  private function validIntegrity() {
    $result = $this->query("select", "vars", array("WHERE k='%s'"=>"round"));
    $result1 = $this->query("select", "vars", array("WHERE k='%s'"=>"nextround"));
    $result2 = $this->query("select", "vars", array("WHERE k='%s'"=>"votescleared"));
    if($result[0]['v'] < 1)
      return false;
    if($result1[0]['v']-time() < 0)
      return false;
    if($result2[0]['v'] < 0 || $result2[0]['v'] > 1)
      return false;
    return true;
  }
  private function pickWinner() {
    $tmpvotes = $this->query("select", "tmpvotes");
    $eventscore = array();
    foreach($tmpvotes as $vote) {
      if(!isset($eventscore[$vote['eventid']]))
        $eventscore[$vote['eventid']] = 1;
      else
        $eventscore[$vote['eventid']]++;
    }
    if(count($eventscore) > 0) {
      asort($eventscore, SORT_NUMERIC);
      $eventscore = array_reverse($eventscore, true);
      $samescore = array();
      $maxfound = reset($eventscore);
      foreach($eventscore as $k=>$score) {
        if($score >= $maxfound) {
          $samescore[] = array("id"=>$k, "score"=>$score);
          $maxfound = $score;
        }
      }
      if(count($samescore) > 1) {
        $rand = rand(0, count($samescore)-1);
        $winner = $samescore[$rand];
      } else $winner = end($samescore);
    } else {
      $winner = $this->randomWinner();
    }
    $filepath = $this->eventIdFile($winner['id']);
    if($filepath == null) {
      #die("Fatal error: file could not be found -can't share the winner .pkm file!");
      $filepath = "";
    }
    $setpkmfile = $this->queryInsUpd("vars", array("k", "v"), array("pkmfile", $filepath), array("k", "pkmfile"));
    if($this->queryOk($setpkmfile)) {
      $this->logWinner($winner);
      return true;
    }
    return false;
  }
  private function randomWinner($tries=3) {
    $result = $this->query("select", "events");
    if($this->queryOk($result)) {
      $validevents = array();
      foreach($result as $event)
        if($event['active'] == "1")
          $validevents[] = $event['id'];
      $id = rand(1, count($validevents));
      $id = $validevents[$id];
    } else $id = 0; // should not happen...
    while($id == 0 && $tries > 0) {
      $id = $this->randomWinner($tries-1);
      $id = $id['id'];
    }
    if($id == 0)
      $id = $this->firstValidEventID(); // well, if it's that bad then just return the first event that's valid
    return array("id"=>$id, "score"=>-1); // returns [eventid, -1 in score (rand)]
  }
  private function logWinner($data) {
    $log = $this->query("insert", "winnerlog", array("eventid", "score", "time"), array($data['id'], $data['score'], time()));
    return $this->queryOk($log);
  }
  private function eventIdFile($eventid) {
    global $cfg;
    $file = $this->query("select", "events", array("WHERE id='%d'"=>$eventid));
    if($this->queryOk($file)) {
      $relpath = $cfg['distpkm']."/".$file[0]['filepkm'];
      if(file_exists($relpath))
        return $relpath;
      else
        return null;
    }
    return null;
  }
  private function eventDeactivate($pkmfile) {
    $result = $this->query("select", "events", array("WHERE filepkm='%s'"=>$pkmfile));
    if($this->queryOk($result))
      if(!empty($result[0]['filepkm']) && $result[0]['active'] != "0")
        $this->queryInsUpd("events", array("filepkm", "active"), array($result[0]['filepkm'], "0"), array("id", $result[0]['id']));
  }
  private function eventActivate($pkmfile) {
    $result = $this->query("select", "events", array("WHERE filepkm='%s'"=>$pkmfile));
    if($this->queryOk($result))
      if(!empty($result[0]['filepkm']) && $result[0]['active'] != "1")
        $this->queryInsUpd("events", array("filepkm", "active"), array($result[0]['filepkm'], "1"), array("id", $result[0]['id']));
  }
  private function firstValidEventID() {
    $result = $this->query("select", "events", array("WHERE active='%d'"=>1));
    return !empty($result[0]['id']) ? $result[0]['id'] : -1; // now we are screwed :(
  }
}
?>