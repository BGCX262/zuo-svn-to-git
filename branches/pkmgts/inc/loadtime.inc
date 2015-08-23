<?php
class LoadTime {
  private $start, $end, $spend;
  public function __construct() {
    $this->start = time();
  }
  public function calc() {
    $this->end = time();
    $spend = $this->end - $this->start;
    return sprintf("%01.2fs", $spend);
  }
}
?>