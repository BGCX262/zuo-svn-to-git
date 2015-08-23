<?php
session_name("GTS_GlobalTradeStation");
//session_cache_limiter("private_no_expire");
session_cache_expire(32140800); // 1 year (in minutes)
session_start();

function session_clean() {
  return session_destroy();
}
?>