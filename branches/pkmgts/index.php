<?php
// includes
include_once "config.php";
include_once "inc/loadtime.inc";
$load = new LoadTime(); // start counting
include_once "inc/funcs.inc";
include_once "inc/database.inc";
include_once "inc/gts.inc";
include_once "inc/sessions.inc";

// connect to database if there is no connection yet
if($db == null)
  $db = new Database();

// insert current distrobution pkm file (or empty if none can be found)
$cfg['distpkm'] = currentDistro();

// declare variables
$html = "";

// fetch latest (or create) GTS class
$gts = getGTS();
if($gts->handleGET($_GET)) {
  $gts->prepareHeaders();
  $ret = !isset($cfg['disabled']) || empty($cfg['disabled']) ? $gts->output() : "";
  echo $ret;
  if($cfg['gtslogging']) {
    $obraw = ob_get_contents();
    file_put_contents("./gts_".(countLogs()+1).".log", $obraw);
    ob_end_flush();
  }
  die();
}

// generate site
$cfg['title'] = "";
if(!isset($cfg['disabled']) || empty($cfg['disabled'])) {
  $distpkm = $db->query("select", "vars", array("WHERE k='%s'"=>"pkmfile"));
  if($db->queryOk($distpkm)) {
    $distpkm = $distpkm[0]['v'];
    $cfg['distpkm'] = $distpkm;
  } elseif(!file_exists($cfg['distpkm']))
    $cfg['distpkm'] = "";
  $html .= handleGET($_GET);
} else
  $cfg['gtsonline'] = false; // site offline logo

// output html
@header("Content-Type: text/html; charset=UTF-8");
#<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
  "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<link rel="stylesheet" type="text/css" href="./main.css"/>
<!--[if IE]>
<link rel="stylesheet" type="text/css" href="./main_ie.css"/>
<![endif]-->
<script type="text/javascript" src="./jquery-1.4.2.min.js"></script>
<script type="text/javascript" src="./jquery-countdown.min.js"></script>
<script type="text/javascript" src="./jquery-tablesorter.min.js"></script>
<script type="text/javascript" src="./core.js"></script>
<title>Global Trade Station<?php echo !empty($cfg['title']) ? " :: {$cfg['title']}" : "";?></title>
<?php
// show countdown code and refresh site code (vote related) only when certain conditions are met
if($cfg['gtsonline'] && (isset($_GET['archive']) || !isset($_GET['howto']))) {
?>
<script type="text/javascript">
// <![CDATA[
$(document).ready(function() {
<?php
if(isset($_GET['archive'])) // only if in archive
  echo '  $("#roundcd").countdown({until: $.countdown.UTCDate(2, '.roundCountdownJS().')});'."\n";
?>
  setTimeout('nextRound()', <?php echo roundCountdownJS(1);?>);
});
// ]]>
</script>
<?php
}
?>
</head>
<body>
<div id="img"></div>
<noscript>
  <p>Sorry, you need Javascript in order to use this site. Please hit F1 and look-up how you enable it -good luck!</p>
</noscript>
<div id="wrap">
  <div id="head" class="abo"><a class="nojs" href="./"><img src="./img/logo<?php echo !$cfg['gtsonline'] ? "2" : "";?>.png" alt="Vlad's Global Trade Station"/></a></div>
  <div id="bord" class="cen">
<?php
if(!isset($cfg['disabled']) || empty($cfg['disabled'])) {
?>
    <div id="nav">
      <ul>
        <li><a class="nojs" href="./">News</a></li>
        <li><a class="nojs" href="./?archive">Event archive</a></li>
        <li><a class="nojs" href="./?howto">How to connect</a></li>
<?php
echo !$cfg['gtsonline'] ?
  "        <li class=\"offline\">The GTS is offline.</li>\n" :
  "        <li class=\"online\">".(!empty($cfg['gtsonlinesince']) && $cfg['gtsonlinesince']>0 ? "GTS has been online for ".timeDiff($cfg['gtsonlinesince'], 1) : "The GTS is online").".</li>";
?>
      </ul>
    </div>
    <div id="cont"><?php echon(!empty($cfg['mysqlfatal']) ? "<b>MySQL error:</b> <em>{$cfg['mysqlfatal']}</em>" : $html, 1);?></div>
<?php
} else {
  echo "    <p>{$cfg['disabled']}</p>\n";
}
?>
  </div>
  <div id="foot"><p>Created by Vlad for the Project Pokemon community. Thanks to Rokis17 and SneakyTomato for their art. Generated in <?php echo $load->calc();?><span id="jsload"></span>. <a href="http://validator.w3.org/check?uri=referer" rel="ext"><img src="./img/xhtml.png" alt="XHTML 1.0"/></a></p></div>
</div>
</body>
</html>
<?php
#session_clean();
?>