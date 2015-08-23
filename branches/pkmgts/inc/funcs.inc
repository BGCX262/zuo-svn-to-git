<?php
function handleGET($req) {
  global $cfg;
  $html = "";
  // requesting XML?
  if(isset($req['xml']) && count($req) == 1) {
    header("Content-Type: text/xml; charset=utf-8");
    $nextround = getVar("nextround");
    $timeleft = timeDiff($nextround, 0, 1);
    echo '<?xml version="1.0" encoding="utf-8"?>'."\n";
    echo "<gts>\n";
    echo "  <round>\n";
    echo "    <id>".currentRound()."</id>\n";
    echo "    <started>".($nextround-(60*$cfg['roundtime']))."</started>\n";
    echo "    <ends>{$nextround}</ends>\n";
    echo "    <timeleft>\n";
    echo "      <days>{$timeleft['days']}</days>\n";
    echo "      <hours>{$timeleft['hours']}</hours>\n";
    echo "      <minutes>{$timeleft['minutes']}</minutes>\n";
    echo "      <seconds>{$timeleft['seconds']}</seconds>\n";
    echo "    </timeleft>\n";
    echo "  </round>\n";
    echo "  <event>\n";
    echo currentWinner(1);
    echo "  </event>\n";
    echo "</gts>";
    die();
  // requesting JSON?
  } elseif(isset($req['json']) && count($req) == 1) {
    header("Content-Type: text/json; charset=utf-8");
    $nextround = getVar("nextround");
    $timeleft = timeDiff($nextround, 0, 1);
    $array = array(
      "gts"=>array(
        "round"=>array(
          "id"=>currentRound(),
          "started"=>$nextround-(60*$cfg['roundtime']),
          "ends"=>$nextround,
          "timeleft"=>array(
            "days"=>$timeleft['days'],
            "hours"=>$timeleft['hours'],
            "minutes"=>$timeleft['minutes'],
            "seconds"=>$timeleft['seconds'],
          ),
        ),
        "event"=>array(/* (added 4 lines below) */),
      ),
    );
    $array['gts']['event'] = @array_merge($array['gts']['event'], currentWinner(2));
    $json = @json_encode($array);
    echo $json;
    die();
  }
  // interpret vote queries
  if(!empty($req['vote'])) {
    if(!$cfg['gtsonline']) {
      @header("Location: ./?archive");
      $html .= "<p>The server is offline thus voting has been disabled. <a class=\"nojs\" href=\"./?archive\">Click here</a> to return.</p>";
    } else {
      if(legitVote($req['vote'])) {
        if(voteFor($req['vote'])) {
          @header("Location: ./?archive");
          $html .= "<p>Your vote for ".eventFromID($req['vote'], "name")." has been recorded, <a class=\"nojs\" href=\"./?archive\">please click here to return</a>.</p>";
        } else
          $html .= "<p>Sorry but there has been a problem and your vote could not be stored. <a class=\"nojs\" href=\"./?archive\">Click here to return</a> and try again.</p>";
      } else {
        @header("Location: ./?archive");
        $html .= "<p>Sorry but the vote is invalid, <a class=\"nojs\" href=\"./?archive\">click here to return</a>.</p>";
      }
    }

  // show events page
  } elseif(isset($req['archive'])) {
    if(isset($req['winner'])) {
      $_SESSION['evtwinner'] = time();
      die(); // jquery get, no need to load the server with useless queries
    }
    if($cfg['gtsonline']) {
      $html .= "<p>This is the ".currentRound()." round and the next round starts in:</p>";
      $html .= "<p><span id=\"roundcd\"></span></p>";
      if(isset($_SESSION['evtwinner'])) {
        $winner = currentWinner(3);
        $html .= "<div id=\"evtwinner\" class=\"clear\">";
        $html .= "<img class=\"pkmimg\" src=\"".sprintf($cfg['pokedeximgurl'], $winner[1]['pkmid'])."\" alt=\"\">";
        $html .= "<h1>And the winner is... {$winner[1]['pkmname']}!</h1>";
        $html .= "<p>From {$winner[1]['pkmot']} (".(!empty($winner[1]['extra']) ? (substr($winner[1]['extra'], strlen($winner[1]['extra'])-1) == "." ? substr($winner[1]['extra'], 0, strlen($winner[1]['extra'])-1) : $winner[1]['extra']) : "No description").")</p>";
        $html .= "<p class=\"foot\"><em>".($winner[0]['score']==-1 ? "Automatic pick by the server, hope you like it!" : "Won with {$winner[0]['score']} votes!")."<br/><span class=\"gray\">Announcement will be dismissed in <span class=\"cd\">15</span> seconds...</span></em></p>";
        $html .= "</div>";
        unset($_SESSION['evtwinner']);
      }
      $html .= "<p class=\"forie clear\">I know the timer looks buggy in Internet Explorer, will fix it when I can.</p>";
      $charturl = googleChart();
      if(!empty($charturl))
        $html .= "<p class=\"clear\"><img src=\"{$charturl}\" alt=\"Loading chart...\"/></p>";
      #else $html .= "<p class=\"clear\"><em>There has not been any votes yet, why not be the first?</em></p>";
    } else
      $html .= "<p>The server is currently offline, please try again later.</p>";
    $html .= "<h3 class=\"clear\">Pokémon available for nomination</h3>";
    $html .= buildEventTable();

  // show "how to connect" guide
  } elseif(isset($req['howto'])) {
    $html .= "<ol class=\"guide\">";
    $html .= "<li>Start the game and go to the title screen.</li>";
    $html .= "<li>Select &quot;Nintendo WFC settings&quot; from the main menu.</li>";
    $html .= "<li>Tap &quot;Nintendo Wi-Fi Connection Settings&quot;.</li>";
    $html .= "<li>Select the connection you would like to use (select a working connection).</li>";
    $html .= "<li><em>Optional: Erase the connection to set it up again -this way you make sure it works.</em></li>";
    $html .= "<li>Once you open the connection settings, scroll down to &quot;Auto-obtain DNS&quot; and set this to &quot;No&quot;.</li>";
    $html .= "<li>Edit &quot;Primary DNS&quot; and set it to <span class=\"code\">{$cfg['dnsip']}</span>.</li>";
    $html .= "<li><em>Optional: You can set &quot;Secondary DNS&quot; to your routers gateway, <a href=\"javascript:;\" onclick=\"alert('Because when this DNS server is offline you may still use the official GTS. The NDS will see that the primary DNS is offline and use the secondary DNS. If both fail then you obviously won\'t be going anywhere.');\">why?</a></em></li>";
    $html .= "<li>Save the settings and load your save-file, grats you are done!</li>";
    $html .= "</ol>";
    $html .= "<h3>Screenshots</h3>";
    $html .= "<p class=\"guide\">";
    for($i=1; $i<=6; $i++)
      $html .= "<a class=\"nojs\" href=\"#\"><img src=\"./img/dnsguide/{$i}.jpg\" alt=\"Loading screenshot {$i}...\"/></a>";
    $html .= "</p>";
    $html .= "<p class=\"center\"><img id=\"imgfocus\" class=\"hide border\" src=\"\" alt=\"\"/></p>";

  } else {
    $html .= "<p>What is this? It's a custom made Global Trade Station that any Pokémon game (with GTS support) can access.</p>";
    #basically everything you need to know about connecting to this server is explained in the &quot;How to connect&quot; page.
    $html .= "<p>This is how this works; when you connect to the server you will instantly receive the winner Pokémon from the last nomination round, it will be located in your party/storage box!</p>";
    $html .= "<p>How do you change what Pokémon you receive you ask? Frankly this is a community driven server, meaning that what the majority wants is what the majority gets. Vote for the one you want the most and fifteen minutes later you will find out if it won or not. A new round starts each {$cfg['roundtime']} minutes, meaning that the most popular one is the one being shared. If no one votes there will be a randomly picked winner so it's not like it's going to be disabled or anything.</p>";
    $roundcount = currentRound();
    if($roundcount-1 > 0) {
      $html .= "<h3>The winner of round ".($roundcount-1)." was...</h3>";
      $html .= currentWinner();
      $html .= "<h3>Previous winners</h3>";
      $html .= pastWinners();
    }
  }
  return $html;
}

function publicIP($ip) {
  if($ip == "127.0.0.1")
    return false; // for performance sake ;)
  $localaddr = array( // list over local IP ranges (routers)
    array(array( 10,   0, 0, 0), array( 10, 255, 255, 255)), // 10.0.0.0-10.255.255.255 (16,777,216)
    array(array(172,  16, 0, 0), array(172,  31, 255, 255)), // 172.16.0.0-172.31.255.255 (1,048,576)
    array(array(192, 168, 0, 0), array(192, 168, 255, 255)), // 192.168.0.0-192.168.255.255 (65,536)
  );
  $parts = explode(".", $ip);
  if(count($parts) == 4)
    foreach($localaddr as $localdata) {
      $false = 0;
      for($i=0; $i<4; $i++)
        if($parts[$i] >= $localdata[0][$i] && $parts[$i] <= $localdata[1][$i])
          $false++;
      if($false == 4)
        return false;
    }
  return true;
}

function endsWith($redir, $file) {
  return substr($redir, strlen($redir)-strlen($file)) == $file;
}

function utf8($obj, $ucfirst=0) {
  if(is_array($obj)) {
    $clean = array();
    foreach($obj as $k=>$v)
      $clean[$k] = utf8($v, $ucfirst);
  } else {
    $clean = $obj;
    $curenc = mb_detect_encoding($clean);
    if(!($curenc == "UTF-8" && mb_check_encoding($clean, "UTF-8")))
      $clean = utf8_encode($clean);
  }
  return $ucfirst && !is_array($clean) ? ucfirst(strtolower($clean)) : $clean;
}

function timeDiff($start=-1, $asstr=0, $rev=0) {
  //$start = time()-3600;
  $end = time();
  if($rev) {
    $tmp = $end;
    $end = $start;
    $start = $tmp;
    unset($tmp);
  }
  $uts = array();
  $uts['start'] = $start;
  $uts['end'] = $end;
  if($uts['start'] !== -1 && $uts['end'] !== -1) {
    if($uts['end'] >= $uts['start']) {
      $diff = $uts['end'] - $uts['start'];
      if($days = intval((floor($diff/86400))))
        $diff = $diff % 86400;
      if($hours = intval((floor($diff/3600))))
        $diff = $diff % 3600;
      if($minutes = intval((floor($diff/60))))
        $diff = $diff % 60;
      $diff = intval($diff);
      if($asstr) {
        $asstr = ($days>0 ? "{$days} days" : "").
                 ($hours>0 ? ($hours==1 ? ", {$hours} hour" : ", {$hours} hours") : "").
                 ($minutes>0 ? ($minutes==1 ? ", {$minutes} minute" : ", {$minutes} minutes") : "").
                 " and {$diff} ".($diff==1 ? "second" : "seconds");
        if(substr($asstr, 0, 2) == ", ")
          $asstr = substr($asstr, 2);
        if(substr($asstr, 0, 5) == " and ")
          $asstr = substr($asstr, 5);
        return $asstr;
      }
      return array(
        'days'=>$days,
        'hours'=>$hours,
        'minutes'=>$minutes,
        'seconds'=>$diff,
      );
    }
  }
}

function echon($str, $dosmth) { // TODO: can't remember what "$dosmth" flag was for, hmm...
  if(substr($str, strlen($str)-1) == "\n") {
    echo "{$str}\n";
  }
  echo $str;
}

function is_digits($num) {
  return preg_match("/[0-9]*/", $num) && is_numeric($num); // TODO: Obscolete?
}

function roundCountdownJS($rem=0) {
  global $db;
  $result = $db->query("select", "vars", array("WHERE k='%s'"=>"nextround"));
  if($db->queryOk($result)) {
    $result = $result[0]['v'];
    if($rem) // output seconds left for setTimeout(...)
      return $result-time() < 1 ? 2500 : ($result-time()+1)*1000; // if below 1, wait 1 second. if above output time difference in secods but add 1 more second (sort of a time out period)
    $jstime = ""; // output datestamp for new Date(...)
    $jstime .= (date("Y", $result)+0).", ";
    $jstime .= (date("n", $result)-1).", ";
    $jstime .= (date("j", $result)+0).", ";
    $jstime .= (date("G", $result)+0).", ";
    $jstime .= (date("i", $result)+0).", ";
    $jstime .= (date("s", $result)+0);
    return $jstime;
  }
}

function currentRound() {
  global $db;
  $result = $db->query("select", "vars", array("WHERE k='%s'"=>"round"));
  if($db->queryOk($result))
    return $result[0]['v'];
}

function googleChart() {
  global $db;
  $votes = array();
  $result = $db->query("select", "tmpvotes");
  if($db->queryOk($result)) {
    foreach($result as $data) {
      if(!isset($votes[$data['eventid']]))
        $votes[$data['eventid']] = 1;
      else
        $votes[$data['eventid']]++;
    }
    if(count($votes) > 0) {
      $labels = ""; $labels2 = ""; $values = "";
      $outcount = 10; // counts down (max 10)
      asort($votes, SORT_NUMERIC); // sort ASC
      $votes = array_reverse($votes, true); // now sort DESC
      // create the labels and values we pass onto google
      foreach($votes as $eventid=>$votecount) {
        if($outcount < 1)
          break;
        $pkmname = eventFromID($eventid, "name");
        $pkmot = eventFromID($eventid, "pkmot");
        $labels .= urlencode($pkmname)."|";
        #$labels2 .= urlencode(eventFromID($eventid, "name"))." ({$eventid})|";
        $labels2 .= urlencode("{$pkmname} (E{$eventid}) from {$pkmot}")."|";
        $values .= $votecount.",";
      }
      // cleanup endings
      if(!empty($labels))
        $labels = substr($labels, 0, strlen($labels)-1);
      if(!empty($labels2))
        $labels2 = substr($labels2, 0, strlen($labels2)-1);
      if(!empty($values))
        $values = substr($values, 0, strlen($values)-1);
      // build image url
      $imgurl = "http://chart.apis.google.com/chart?cht=p";
      $imgurl .= "&amp;chs=".(empty($labels2) ? "380" : "580")."x220";
      $imgurl .= "&amp;chd=t:{$values}";
      $imgurl .= "&amp;chl={$labels}";
      if(!empty($labels2)) // debugging, displays each events id
        $imgurl .= "&amp;chdl={$labels2}";
      $imgurl .= "&amp;chco=3090FF,FFFFFF"; // "FFFFFF" for a starting point or "FFFF10,FF0000" for specific gradient
      $imgurl .= "&amp;chf=bg,s,FFFFFF95|c,s,FFFFFF95"; // background color and such
      if(!empty($labels2))
        $imgurl .= "&amp;chma=2,2,2,2|200,0"; // margins (left,right,top,bottom|legw,legh)
      else
        $imgurl .= "&amp;chma=2,2,2,2"; // margins (left,right,top,bottom)
      return $imgurl;
    }
  }
}

function legitVote($num) {
  global $db;
  if(is_digits($num)) {
    $ref = !empty($_SERVER['HTTP_REFERER']) ? $_SERVER['HTTP_REFERER'] : "";
    if(!preg_match("/^(.*?)\?archive$/i", $ref))
      return false; // only accept vote if referer is the ?archive site!
    $result = $db->query("select", "events", array("WHERE id='%d'"=>$num));
    if($db->queryOk($result) && count($result) > 0)
      if($result[0]['active'] == "1") // also check that it is an active event
        return true;
  }
  return false;
}

function eventFromID($id, $field="") {
  global $db;
  $result = $db->query("select", "events", array("WHERE id='%d'"=>$id));
  if($db->queryOk($result))
    if(empty($field))
      return $result[0];
    else
      return $result[0][$field];
  return "Unknown";
}

function voteFor($eventid) {
  global $db; // this makes sure there are no duplicates (by using users IP) so people can change their minds :)
  return $db->queryInsUpd("tmpvotes", array("ip", "eventid", "time"), array($_SERVER['REMOTE_ADDR'], $eventid, time()), array("ip", $_SERVER['REMOTE_ADDR']));
}

function buildEventTable() {
  global $db, $cfg;
  $result = $db->query("select", "events", array("WHERE active='%d'"=>1));
  if($db->queryOk($result)) {
    $tmpvoteid = $db->query("select", "tmpvotes", array("WHERE ip='%s'"=>$_SERVER['REMOTE_ADDR']));
    if($db->queryOk($tmpvoteid) && count($tmpvoteid) == 1)
      $tmpvoteid = $tmpvoteid[0]['eventid'];
    else unset($tmpvoteid);
    $html = "<table class=\"tablesorter pkm\">";
    $html .= "<thead>";
    $html .= "<tr>";
    $html .= "<th class=\"vote\">Vote</th>";
    $html .= "<th>Pokemon</th>";
    $html .= "<th>OT</th>";
    $html .= "<th>Level</th>";
    $html .= "<th>Item</th>";
    $html .= "<th>Retired</th>";
    $html .= "<th>Misc.</th>";
    $html .= "</tr>";
    $html .= "</thead>";
    $html .= "<tbody>";
    foreach($result as $k=>$v) {
      foreach($v as $k2=>$v2)
        if(empty($v2))
          $v[$k2] = "-";
      $html .= "<tr>";
      if($cfg['gtsonline'])
        $html .= "<td class=\"vote".(isset($tmpvoteid)&&$tmpvoteid!=$v['id'] ? " ignored" : "")."\"><a class=\"vote\" href=\"./?vote={$v['id']}\"><img src=\"./img/vote.gif\" alt=\"+\"/></a></td>";
      else
        $html .= "<td class=\"vote\"><img src=\"./img/cancel.gif\" alt=\"Disabled\"/></td>";
      $html .= "<td><a class=\"nojs\" href=\"{$cfg['pokedexurl']}{$v['pkmid']}\" rel=\"ext\">{$v['pkmname']}</a></td>";
      $html .= "<td>{$v['pkmot']}</td>";
      $html .= "<td>{$v['pkmlvl']}</td>";
      $html .= "<td>{$v['pkmitem']}</td>";
      $html .= "<td>{$v['avto']}</td>";
      $html .= "<td>E{$v['id']}:".(!empty($v['extra'])&&$v['extra']!="-" ? " {$v['extra']}" : " <em>No description.</em>")."</td>";
      $html .= "</tr>";
    }
    $html .= "</tbody>";
    $html .= "</table>";
    return $html;
  }
  return "<p>A fatal database error has occured and the table could not be build.</p>";
}

function currentWinner($ascode=0) {
  global $db, $cfg;
  $xml = "";
  $html = "";
  $array = array();
  $winner = $db->query("select", "winnerlog", null, null, array("id"=>"DESC"), 1);
  if($db->queryOk($winner)) {
    $winner = $winner[0];
    $score = $winner['score'];
    $timestamp = $winner['time'];
    $eventdata = eventFromID($winner['eventid']);
    $html .= "<table class=\"evt\">";
    $html .= "<tbody>";
    $html .= "<tr>";
    $html .= "<td class=\"title\" colspan=\"4\">{$eventdata['name']}</td>";
    $html .= "</tr>";
    $html .= "<tr>";
    $html .= "<td class=\"desc\" colspan=\"4\">".(!empty($eventdata['extra']) ? $eventdata['extra'] : "No description.")."</td>";
    $html .= "</tr>";
    $html .= "<tr>";
    $html .= "<td class=\"ftitle\">Pokémon</td>";
    $html .= "<td><a class=\"nojs\" href=\"{$cfg['pokedexurl']}{$eventdata['pkmid']}\" rel=\"ext\">{$eventdata['pkmname']}</a></td>";
    $html .= "<td class=\"ftitle\">Level</td>";
    $html .= "<td>".(!empty($eventdata['pkmlvl']) ? $eventdata['pkmlvl'] : "-")."</td>";
    $html .= "</tr>";
    $html .= "<tr>";
    $html .= "<td class=\"ftitle\"><acronym title=\"National Pokédex ID\">Dex</acronym></td>";
    $html .= "<td>".(!empty($eventdata['pkmid']) ? $eventdata['pkmid'] : "-")."</td>";
    $html .= "<td class=\"ftitle\">Held Item</td>";
    $html .= "<td>".(!empty($eventdata['pkmitem']) ? $eventdata['pkmitem'] : "-")."</td>";
    $html .= "</tr>";
    $html .= "<tr>";
    $html .= "<td class=\"ftitle\">OT</td>";
    $html .= "<td>".(!empty($eventdata['pkmot']) ? $eventdata['pkmot'] : "-")."</td>";
    $html .= "<td class=\"ftitle\"><acronym title=\"The date the official distrobution ended, thus no longer obtainable.\">Officially retired</acronym></td>";
    $html .= "<td>".(!empty($eventdata['avto']) ? $eventdata['avto'] : "-")."</td>";
    $html .= "</tr>";
    $html .= "<tr>";
    $html .= "<td class=\"foot\" colspan=\"4\">".($score>-1 ? "Won with {$score} votes -grats!" : "None were nominated, server picked this by random.")."</td>";
    $html .= "</tr>";
    $html .= "</tbody>";
    $html .= "</table>";
    $xml .= "    <title><![CDATA[{$eventdata['name']}]]></title>\n";
    $xml .= "    <extra><![CDATA[{$eventdata['extra']}]]></extra>\n";
    $xml .= "    <name><![CDATA[{$eventdata['pkmname']}]]></name>\n";
    $xml .= "    <natid><![CDATA[{$eventdata['pkmid']}]]></natid>\n";
    $xml .= "    <ot><![CDATA[{$eventdata['pkmot']}]]></ot>\n";
    $xml .= "    <level><![CDATA[{$eventdata['pkmlvl']}]]></level>\n";
    $xml .= "    <helditem><![CDATA[{$eventdata['pkmitem']}]]></helditem>\n";
    $xml .= "    <retired><![CDATA[{$eventdata['avto']}]]></retired>\n";
    $array['title'] = $eventdata['name'];
    $array['extra'] = $eventdata['extra'];
    $array['name'] = $eventdata['pkmname'];
    $array['natid'] = $eventdata['pkmid'];
    $array['ot'] = $eventdata['pkmot'];
    $array['level'] = $eventdata['pkmlvl'];
    $array['helditem'] = $eventdata['pkmitem'];
    $array['retried'] = $eventdata['avto'];
  } else $html .= "<p><em>There has not been a winner yet, that or we experience database issues.</em></p>";
  if($ascode == 1)
    return $xml;
  elseif($ascode == 2)
    return $array;
  elseif($ascode == 3)
    return array($winner, $eventdata);
  return $html;
}

function pastWinners() {
  global $db, $cfg;
  $html = "";
  $showmax = 5; // max visible
  $winners = $db->query("select", "winnerlog", null, null, array("id"=>"DESC"));
  if($db->queryOk($winners)) {
    $html .= "<table class=\"tablesorter reverse history\">";
    $html .= "<thead>";
    $html .= "<tr>";
    $html .= "<th>ID</th>";
    $html .= "<th class=\"center2\">Time</th>";
    $html .= "<th class=\"center2\">Votes</th>";
    $html .= "<th>Event</th>";
    $html .= "<th>Pokémon</th>";
    $html .= "<th>Description</th>";
    $html .= "</tr>";
    $html .= "</thead>";
    $html .= "<tbody>";
    foreach($winners as $winner) {
      if($winner['score'] < 0) // i.e. "-1"
        continue; // skip automated picks
      $eventdata = eventFromID($winner['eventid']);
      $html .= "<tr".(--$showmax < 0 ? " class=\"morehidden\"" : "").">";
      $html .= "<td>{$winner['id']}</td>";
      $html .= "<td class=\"center\">".date("d.m @ H:i", $winner['time'])."</td>";
      $html .= "<td class=\"center\">".($winner['score']<0 ? "Auto" : $winner['score'])."</td>";
      $html .= "<td>{$eventdata['name']}</td>";
      $html .= "<td><a href=\"{$cfg['pokedexurl']}{$eventdata['pkmid']}\" rel=\"ext\">{$eventdata['pkmname']}</a></td>";
      $html .= "<td>".(!empty($eventdata['extra']) ? $eventdata['extra'] : "<em>No description.</em>")."</td>";
      $html .= "</tr>";
    }
    $html .= "</tbody>";
    $html .= "</table>";
    $html .= "<p class=\"more\"><a id=\"showmore\" href=\"javascript:;\">Toggle the visibility of older winners</a></p>";
  }
  return $html;
}

function currentDistro() {
  global $db;
  $result = $db->query("select", "vars", array("WHERE k='%s'"=>"pkmfile"));
  if($db->queryOk($result)) {
    if(!empty($result[0]['v']))
      return $result[0]['v'];
  }
  return "";
}

function getVar($fname) {
  global $db;
  $result = $db->query("select", "vars", array("WHERE k='%s'"=>$fname));
  if($db->queryOk($result)) {
    if(!empty($result[0]['v']))
      return $result[0]['v'];
  }
  return "";
}

function setVar($fname, $value) {
  global $db;
  $return = $db->queryInsUpd("vars", array("k", "v"), array($fname, $value), array("k", $fname));
  return $db->queryOk($return);
}

function pingdns($ip="") {
  global $db;
  if($db == null)
    $db = new Database();
  if(time()-getVar("dnscheck") < 120) // only check every 2 minutes!
    return true;
  setVar("dnscheck", time());
  $dom = nslookup($ip);
  if(!empty($dom) && preg_match("/(.*?)\.(.*?)\.(.*?)/i", $dom))
    return true;
  return false;
}

function nslookup($ip="") {
  if(empty($ip))
    return;
  $cmd = 'nslookup -timeout=1 -retry=2 "'.$ip.'"';
  $res = @`$cmd`;
  if(empty($res)) {
    @exec($cmd, $res2);
    $res = "";
    if(isset($res2) && is_array($res2))
      foreach($res2 as $line)
        $res .= $line."\n";
  }
  if(preg_match("/(Name:(.*?)\n|name = (.*?)\n)/i", $res, $dom))
    return trim(end($dom));
}
?>