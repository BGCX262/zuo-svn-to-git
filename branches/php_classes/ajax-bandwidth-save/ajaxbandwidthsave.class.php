<?php
/**
 * When the library is included in the document, we start the outputbuffer so
 * anything echoed later on can be removed when outputDocument() is used.
 * Otherwise data may not get properly compressed.
 * Note: include the class on the top of the document to avoid issues.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You can find the online version of the GPL version 2 at:
 *   http://www.gnu.org/licenses/gpl-2.0.html
 */
ob_start();

/**
 * 
 *
 * @author Vlad
 */
class AjaxBandwidthSave {
  private $data;
  private $nocomp;
  /**
   * When creating the class, pass down the data you wish to output.
   * Note that non-UTF8 data will be converted to UTF8, may cause problems
   * if you don't take this into account.
   * @param <Object> $data Typically an array or a simple string.
   * @param <Boolean> $nocomp Set this to false to avoid compressing. Usefull
   *   when the data is short, compressing would not really save much bandwidth.
   */
  public function __construct($data, $nocomp=0) {
    $this->data = $this->convert_to_utf8($data);
    $this->nocomp = $nocomp;
  }

  /**
   * Convert anything into proper UTF8 encoded data.
   * @param <Object> $data Will be parsed and converted to UTF8, works for
   *   arrays, objects and strings.
   * @return <Object> Hopefully the converted data, data is passed trough if
   *   the function could not UTF8 encode the input.
   */
  private function convert_to_utf8($data) {
    if(is_array($data) || is_object($data)) {
      foreach($data as $k=>$v) {
        $data[$this->convert_to_utf8($k)] = $this->convert_to_utf8($v);
      }
    }
    return $data;
  }

  /**
   * Compress data as well as gzcompress can do.
   * @param <Object> $data Any kind of data to compress (preferably string).
   * @param <Integer> $complevel The level between 0-9 where 9 is the best.
   * @return <Object> The compressed data.
   */
  private function compress($data) {
    if($this->nocomp) {
      return $data;
    }
    return gzcompress(print_r($data, 1));
  }

  /**
   * Outputs the parameter, uses gzhandler to compress the data and removes
   * junk data from the packet to save bandwidth.
   * @param <Object> $data What ever data you need to send.
   */
  private function outputDocument($data, $type=0) {
    ob_end_clean(); // previous output will be removed
    header_remove(); // remove all set headers
    if($this->nocomp) {
      switch($type) {
        case 1:
          header("Content-Type: text/xml; charset=UTF-8");
          echo '<?xml version="1.0" encoding="UTF-8"?>';
          break;
      }
    }
    echo $data;
    ob_end_flush(); // send the data
    die();
  }

  /**
   * Convert array/object into proper XML.
   * @param <Object> $data Can be an array or object, anything else is returned
   * in the tags <string></string> surrounded by CDATA tags.
   */
  private function parseObjectToXML($data, $level=0) {
    if(is_array($data) || is_object($data)) {
      $out = "";
      foreach($data as $k=>$v) {
        $k = is_numeric($k) ? chr(65+$k) : $k; // using chr(65+$k) would make "0" into "A", "1" into "B", and so forth -alternative- "i".$k to output "i0" instead of digit (plain digits will error the XML)
        $k = preg_replace("/[^A-Za-z0-9\-_]/i", "", $k);
        $v = is_array($v) || is_object($v) ? $this->parseObjectToXML($v, $level+1) : "<![CDATA[{$v}]]>";
        $out .= "<{$k}>{$v}</{$k}>";
      }
      if($level == 0) {
        return "<xml>{$out}</xml>";
      }
      return $out;
    } elseif($level == 0) {
      $data = "<![CDATA[".print_r($data, 1)."]]>";
    }
    return "<xml>{$data}</xml>";
  }

  /**
   * Output as JSON (UTF8 encoded).
   * @param <Integer> $opt A bitmask consisting of one of these values or a combination:
   *  JSON_HEX_QUOT  convert " (quote) chars into \u0022
   *  JSON_HEX_TAG  convert < and > (less and greater) chars into \u003C and \u003E
   *  JSON_HEX_AMP  convert & (amp) chars into \u0022
   *  JSON_HEX_APOS  convert ' (apostrophe) chars into \u0027
   *  JSON_FORCE_OBJECT  convert non-associative array as object
   * Documentation at: http://php.net/manual/en/function.json-encode.php
   * @param <Boolean> $get Return or properly output the data.
   */
  public function outputJSON($opt=null, $get=0) {
    $out = json_encode($this->data, $opt);
    $out = $this->compress($out);
    if($get) {
      return $out;
    }
    $this->outputDocument($out);
  }

  /**
   * Output as XML (UTF8 encoded).
   * @param <Boolean> $get Return or properly output the data.
   */
  public function outputXML($get=0) {
    $out = $this->parseObjectToXML($this->data);
    $out = $this->compress($out);
    if($get) {
      return $out;
    }
    $this->outputDocument($out, 1);
  }

  /**
   * Output as PHP "serialize()" so you can "unserialize()" it later.
   * @param <Boolean> $get Return or properly output the data.
   */
  public function outputSerialized($get=0) {
    $out = serialize($this->data);
    $out = $this->compress($out);
    if($get) {
      return $out;
    }
    $this->outputDocument($out);
  }
}
?>