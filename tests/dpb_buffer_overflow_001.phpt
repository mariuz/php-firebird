--TEST--
Connection parameter buffer overflow
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("functions.inc");
set_exception_handler("php_ibase_exception_handler");

(function(){
    ibase_connect("bogus", "bogus", "bogus", str_repeat("utf8", 100));
})();

?>
--EXPECTF--
Fatal error: ibase_connect(): DPB buffer overflow (connection parameters exceed internal buffer size) %a