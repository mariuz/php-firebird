--TEST--
InterBase: connect, close and pconnect
--SKIPIF--
<?php include(__DIR__."/skipif.inc"); ?>
--FILE--
<?php

	require(__DIR__."/interbase.inc");

	ibase_connect($test_base);
	out_table("test1");
	ibase_close();

	$con = ibase_connect($test_base);
	$pcon1 = ibase_pconnect($test_base);
	$pcon2 = ibase_pconnect($test_base);
	ibase_close($con);
	unset($con);
	ibase_close($pcon1);
	unset($pcon1);

	$pcon2 = ibase_connect($test_base); // As of 6.1.1-RC3 ibase_close() really closes the connection
	out_table("test1");

	ibase_close($pcon2);
	unset($pcon2);
?>
--EXPECT--
--- test1 ---
1	test table not created with isql	
---
--- test1 ---
1	test table not created with isql	
---
