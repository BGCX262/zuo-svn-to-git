<?php
class GTS {
  private $vars = array();
  private $timers;
  private $output;
  private $tokens;

  public function __construct() {
    $this->timers = array("start"=>time(), "tick"=>time());
    $this->tokens = array("pid"=>null, "token"=>null, "hash"=>null);
    return true;
  }

  public function output() {
    return $this->output;
  }

  public function handleGET($req) {
    global $cfg;
    $this->output = "";
    $eoutput = "";
    $gtsfile = "";

    // start output buffer if logging is enabled
    if($cfg['gtslogging'])
      ob_start();

    // detect what file we are supposed to emulate
    $redir = $_SERVER['REDIRECT_URL'];
    foreach($cfg['gtsfiles'] as $file)
      if(endsWith($redir, $file)) {
        $gtsfile = $file;
        break;
      }

    // pure "?pid=[0-9]*" requests (expects token in return)
    if(count($req) == 1) {
      if(is_digits($req['pid'])) {
        $this->tokens = clean($this->tokens);
        $this->tokens['pid'] = $req['pid'];
        $this->tokens['token'] = "";
        $pop = array(array(48,57),array(65,90),array(97,122)); // 0-9, A-Z, a-z
        for($i=1; $i<=32; $i++) {
          $subpop = $pop[rand(0, count($pop)-1)];
          $this->tokens['token'] .= chr(rand($subpop[0], $subpop[1]));
        }
        $this->tokens['hash'] = $this->genHash($this->tokens['token']);
        $this->output .= $this->tokens['token'];
        if($cfg['debug'])
          $this->output .= "\nexpects hash: ".$this->tokens['hash'];
      } else $eoutput .= "error: invalid pid";

    // other requests are always pid, hash and data (three in lenght)
    } elseif(count($req) == 3) {
      #if($this->confirmHash($req['hash'])) {
        $this->output .= $this->returnGTS($gtsfile, $req); // return proper GTS response
      #} else {
      #  $this->tokens = clean($this->tokens); // ... it is now!
      #  $eoutput .= "error: token expired";
      #}

    // if nothing matched
    } else $eoutput .= "error: no output generated";

    // log this request if logging is enabled
    if($cfg['gtslogging'])
      gtsLog($gtsfile, $req, $this->output);

    // finalize GTS output
    if($cfg['debug'] && !empty($eoutput))
      $this->output = $eoutput; // errors and debug is on, return them!
    elseif(!empty($gtsfile) && empty($this->output))
      $this->output = "\x01\x00"; // fallback, return 0x01 if no output available

    // update tick and return bool
    if(!empty($this->output)) {
      if($this->output == "null") // null responses are done like this
        $this->output = "";
      @header("Content-Lenght: ".mb_strlen($this->output));
      $this->timers['tick'] = time();
      return true;
    }
    return false;
  }

  public function prepareHeaders() {
    if(headers_sent())
      return false;
    $allowed = array("Cache-Control", "Last-Modified", "Expires", "Pragma", "Content-Lenght");
    if(function_exists("header_remove")) { // new in 5.3.0+ and we can handle a little backward compatibility!
      $headers = apache_response_headers();
      foreach($headers as $k=>$v)
        if(!in_array($k, $allowed))
          header_remove($k);
    }
    header("Cache-Control: no-cache, must-revalidate"); // HTTP/1.1
    header("X-Powered-By: Vlad@projectpokemon.org/forums/member.php?232"); // just leaving my mark ;)
    #print_r(apache_response_headers());die(); // output headers
    return true;
  }

  private function genHash($token) {
    return sha1("sAdeqWo3voLeC5r16DYv{$token}");
  }

  private function confirmHash($hash) {
    return $this->tokens['hash'] == $hash;
  }

  private function officialGTS($file, $req) {
    $data = array();
    while(list($n,$v) = each($req))
      $data[] = $n."=".$v;
    $data = implode("&", $data);
    $gs2request = "http://gamestats2.gs.nintendowifi.net/pokemondpds/{$file}?{$data}";
    $raw = file_get_contents($gs2request);
    return $raw;
  }

  private function returnGTS($file, $req) {
    #print_r($file);print_r($req);die();
    global $cfg;
    $filelow = strtolower($file);
    if(substr($filelow, 0, 11) == "battletower")
      return $this->officialGTS($file, $req);
    switch($filelow) {

      /* Documentation can be found at (WIP):
         http://projectpokemon.org/wiki/GTS_protocol */

      case "common/setprofile.asp":
        #cleanGTStemps(); // assumes a new session has started (only called when connecting to the GTS) TODO: figure out if this is really called once per login or not...
        return str_repeat("\x00", 8);
        break;

      case "worldexchange/info.asp":
        return "\x01\x00";
        break;

      case "worldexchange/result.asp":
        #if(isset($_SESSION['deposited']))
        #  return "\x01\x00";
        if(file_exists($cfg['distpkm'])) {
          if(isset($_SESSION['alreadysent'])) {
            return "\x04\x00"; // a pokemon is deposited on the server
          } return pkmFileToBin($cfg['distpkm']); // returns the pokemon file (properly encoded)
        }
        return "\x05\x00"; // no incoming pokemon
        break;

      case "worldexchange/get.asp":
        if(file_exists($cfg['distpkm'])) {
          return pkmFileToBin($cfg['distpkm']); // odd but again, for fetching the pokemon file
        } return "\x01\x00"; // if nothing then sending ack flag
        break;

      case "worldexchange/return.asp":
        if(file_exists($cfg['distpkm']))
          $_SESSION['alreadysent'] = time();
        return "\x01\x00";
        break;

      case "worldexchange/delete.asp":
        if(file_exists($cfg['distpkm']))
          $_SESSION['alreadysent'] = time();
        return "\x01\x00";
        break;

      case "worldexchange/search.asp":
        return "null"; // no results flag (0 bytes, handled right before output)
        break;

      case "worldexchange/post.asp":
        // _GET['data'] contains the pokemon data (encrypted)
        $_SESSION['deposited'] = time();
        return "\x01\x00"; // block user from depositing
        break;

      case "worldexchange/post_finish.asp":
        return "\x01\x00"; // ack flag
        break;

    }
    return "";
  }
}

function getGTS() {
  if(!isset($_SESSION['gts']))
    $_SESSION['gts'] = new GTS();
  return $_SESSION['gts'];
}

function cleanGTStemps() {
  unset($_SESSION['alreadysent']);
  unset($_SESSION['deposited']);
}

function clean($array) {
  foreach($array as $k=>$v)
    if(!is_array($v))
      $array[$k] = "";
  return $array;
}

function gtsLog($asp, $req, $out) {
  $logfile = "./gts_log-summary.txt";
  $entry = "--transmition_start--\n";
  $entry .= "asp: '{$asp}'\n";
  $entry .= "req: '".serialize($req)."'\n";
  $entry .= "out: '{$out}'\n";
  $entry .= "--end_of_transmition--";
  $log = "";
  if(file_exists($logfile))
    $log .= file_get_contents($logfile)."\n";
  file_put_contents($logfile, $log.$entry);
}

function countLogs() {
  $c = 0;
  $dir = "./";
  if($dh = opendir($dir)) {
    while(($file = readdir($dh)) !== false)
      if(preg_match("/gts_(.*?)\.log$/", $file))
        $c++;
    closedir($dh);
  }
  return $c;
}

function pkmFileToBin($pkmfile) {
  global $cfg;
  if(!file_exists($cfg['pkmlib']))
    return "";
  if(file_exists($pkmfile.".proper"))
    $fname = $pkmfile.".proper";
  else
    $fname = @exec(realpath($cfg['pkmlib'])." \"".realpath($pkmfile)."\"");
  if(!empty($fname) && file_exists($fname)) {
    $bin = file_get_contents($fname);
    #@unlink($fname);
    return $bin;
  }
  return "\x01\x00";
}
?>