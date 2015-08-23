<?php
include_once "ajaxbandwidthsave.class.php";

/* Output the data as JSON but encoded using gzcompress */
$data = "This is where you put the data you wish to send. Short strings should";
$data .= " not be compressed, on the other hand larger data when compressed do";
$data .= " actually save quite a lot of bandwidth. Note that you can transfer";
$data .= " arrays and objects and not only plain strings like this example. :)";
$abs = new AjaxBandwidthSave($data);
$abs->outputJSON();

/* The AjaxBandwidthSave class can be constructed in two ways:
 * 1. new AjaxBandwidthSave($data, 1) - will output plain data (no compression)
 * 2. new AjaxBandwidthSave($data) - will output with compression
 *
 * After that you can do this to output or get the data after it's parsed:
 * 1. $instance->outputJSON($opt, $get) - JSON (UTF8)
 * 2. $instance->outputXML($get) - XML (UTF8) with <xml></xml> tags around the data
 * 3. $instance->outputSerialized($get) - serialize the data so PHP can later unserialize it and get the proper data back
 *
 * $opt - null as default, otherwise a bitmask from: http://php.net/manual/en/function.json-encode.php
 * $get - 0 as default, otherwise 1 for true and will not output but simply return the data
 */

?>