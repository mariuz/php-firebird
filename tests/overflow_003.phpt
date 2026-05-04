--TEST--
Integer overflow tests (32-bit)
--SKIPIF--
<?php
include(__DIR__."/skipif.inc");
if (PHP_INT_SIZE > 4) print "skip: only for 32-bit PHP"
?>
--FILE--
<?php

require(__DIR__."/interbase.inc");
require(__DIR__."/common.inc");

ibase_connect($test_base);

(function() {
    ibase_query(file_get_contents(__DIR__."/001-FIELDS25.sql"));
    ibase_commit();

    $data = [[
        'SMALLINT_1'       => 2**15-1,
        'INTEGER_FIELD'    => PHP_INT_MAX,
        'BIGINT_FIELD'     => "9223372036854775807",
    ], [
        'SMALLINT_1'       => -2**15,
        'INTEGER_FIELD'    => PHP_INT_MIN,
        'BIGINT_FIELD'     => "-9223372036854775808",
    ],[
        'SMALLINT_1'       => 2**15,
        'INTEGER_FIELD'    => "2147483648", // PHP_INT_MAX + 1
        'BIGINT_FIELD'     => "9223372036854775808",
    ],[
        'SMALLINT_1'       => -2**15-1,
        'INTEGER_FIELD'    => "-2147483649", // PHP_INT_MIN - 1
        'BIGINT_FIELD'     => "-9223372036854775809",
    ]];

    $id = 0;
    foreach ($data as $line) {
        foreach ($line as $field => $value) {
            $id++;
            if (ibase_query("INSERT INTO FIELDS25 (ID, $field) VALUES (?, ?)", $id, $value)) {
                $control = ibase_fetch_object(ibase_query("SELECT $field AS VAL FROM FIELDS25 WHERE ID = ?", $id));
                printf("$field => %s (was: %s, got: %s)\n",
                    $value === $control->VAL ? "OK" : "FAIL", $value, $control->VAL);
            } else {
                printf("$field => FAILED to insert %s\n", $value);
            }
        }
    }
})();

?>
--EXPECTF--
SMALLINT_1 => OK (was: 32767, got: 32767)
INTEGER_FIELD => OK (was: 2147483647, got: 2147483647)
BIGINT_FIELD => OK (was: 9223372036854775807, got: 9223372036854775807)
SMALLINT_1 => OK (was: -32768, got: -32768)
INTEGER_FIELD => OK (was: -2147483648, got: -2147483648)
BIGINT_FIELD => OK (was: -9223372036854775808, got: -9223372036854775808)

Warning: ibase_query(): Dynamic SQL Error SQL error code = -303 arithmetic exception, numeric overflow, or string truncation numeric value is out of range %s
SMALLINT_1 => FAILED to insert 32768

Warning: ibase_query(): Dynamic SQL Error SQL error code = -303 arithmetic exception, numeric overflow, or string truncation numeric value is out of range %s
INTEGER_FIELD => FAILED to insert 2147483648

Warning: ibase_query(): Dynamic SQL Error SQL error code = -303 arithmetic exception, numeric overflow, or string truncation numeric value is out of range %s
BIGINT_FIELD => FAILED to insert 9223372036854775808

Warning: ibase_query(): Dynamic SQL Error SQL error code = -303 arithmetic exception, numeric overflow, or string truncation numeric value is out of range %s
SMALLINT_1 => FAILED to insert -32769

Warning: ibase_query(): Dynamic SQL Error SQL error code = -303 arithmetic exception, numeric overflow, or string truncation numeric value is out of range %s
INTEGER_FIELD => FAILED to insert -2147483649

Warning: ibase_query(): Dynamic SQL Error SQL error code = -303 arithmetic exception, numeric overflow, or string truncation numeric value is out of range %s
BIGINT_FIELD => FAILED to insert -9223372036854775809
