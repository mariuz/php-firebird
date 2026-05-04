<?php

if (!extension_loaded("interbase")) print "skip interbase extension not available";
require(__DIR__."/interbase.inc");
if(!@ibase_connect($test_base)){
        die("skip: cannot connect");
}
