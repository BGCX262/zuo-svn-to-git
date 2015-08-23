<?php
#error_reporting(0);

// Please do not change this, it's simply to sync the Javascript and PHP time()
// together so the clients browsing the site see the same countdown timer. ;)
date_default_timezone_set("Europe/Oslo");

include_once "inc/funcs.inc";
include_once "inc/database.inc";

$cfg = array(
  "debug"=>false,
  // log all NDS connections with our GTS?
  "gtslogging"=>false,
  // disable site (empty string or comment out to enable):
  #"disabled"=>"Ten thousand chimps are performing maintenance work on the server, please come visit us again shortly!",
  // mysql settings:
  "mysql"=>array(
    "127.0.0.1", // ip address
    "root",      // username
    "134679",    // password
    "_gts",      // database name
  ),
  // website url:
  "siteurl"=>"http://vlacula.no-ip.com/pokemon/gts/",
  // pokedex url:
  "pokedexurl"=>"http://projectpokemon.org/pokedex/pokedex.php?ver=hgss&amp;lang=e&amp;natid=",
  // pokedex pokemon image url:
  "pokedeximgurl"=>"http://projectpokemon.org/imagedex/hgsssprites/%d_30_0.png", // %d is the pokemon national ID
  // path to pkmlib.py:
  "pkmlib"=>"./backend/pkmlib.py",
  // distrobution system (path to pkm file -relative to pkmlib's location, empty string to disable):
  "distpkm"=>"./backend/pkmfiles",
  // valid .proper filesizes (after the .pkm is encoded) note that invalid sizes will blue screen the game!
  "validpkmpropersizes"=>array( // filesizes in bytes
    #192, // ?
    #193, // ?
    292, // confirmed in HGSS
    #293, // blue screen
    #294, // blue screen
  ),
  // round timer (minutes each round should last)
  "roundtime"=>15,
  // we assume it's online at this point (no point changing)
  "gtsonline"=>true,
  // replace $_SERVER['HTTP_HOST'] with IP if DNS runs on another machine:
  "dnsip"=>gethostbyname($_SERVER['HTTP_HOST']),
  // set to empty string if DNS server runs on another machine (set to "." if it runs from same directory as website):
  "localdnspath"=>"./backend",
  "gtsfiles"=>array(
    "battletower/roomnum.asp",
    "battletower/download.asp",
    "battletower/upload.asp",
    "battletower/info.asp",
    "common/setProfile.asp",
    "worldexchange/post.asp",
    "worldexchange/post_finish.asp",
    "worldexchange/get.asp",
    "worldexchange/result.asp",
    "worldexchange/delete.asp",
    "worldexchange/return.asp",
    "worldexchange/search.asp",
    "worldexchange/exchange.asp",
    "worldexchange/exchange_finish.asp",
    "worldexchange/info.asp",
  ),
);

if(!publicIP($cfg['dnsip'])) // fetch public IP if local one is declared
  $cfg['dnsip'] = file_get_contents("http://folk.uio.no/alexpe/ip");

// check server status (DNS server)
if(!empty($cfg['localdnspath']) && is_dir($cfg['localdnspath'])) {
  // if locally running
  if(!file_exists("{$cfg['localdnspath']}/.lock"))
    $cfg['gtsonline'] = false;
  else
    $cfg['gtsonlinesince'] = filemtime("{$cfg['localdnspath']}/.lock");
} else {
  // if running on external machine, check UDP53 for response
  if(pingdns($cfg['dnsip']))
    $cfg['gtsonlinesince'] = 0; // unknown, hard to tell ;)
}

?>