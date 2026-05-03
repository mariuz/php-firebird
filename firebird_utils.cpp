/*
  +----------------------------------------------------------------------+
  | Copyright (c) The PHP Group                                          |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | https://www.php.net/license/3_01.txt                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Simonov Denis <sim-mail@list.ru>                             |
  | Author: Martins Lazdans <marrtins@dqdp.net>                          |
  +----------------------------------------------------------------------+
*/

extern "C" {
#include <ibase.h>
#include "php.h"
#include "firebird_utils.h"
#include "php_ibase_includes.h"
#include "ext/date/php_date.h"
#include "zend_interfaces.h"
}

#if FB_API_VER >= 30
#include <firebird/Interface.h>
#include <cstring>

/* Returns the client version. 0 bytes are minor version, 1 bytes are major version. */
extern "C" unsigned fbu_get_client_version(void *master_ptr)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::IUtil* util = master->getUtilInterface();
	return util->getClientVersion();
}

extern "C" ISC_TIME fbu_encode_time(void *master_ptr, unsigned hours, unsigned minutes, unsigned seconds, unsigned fractions)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::IUtil* util = master->getUtilInterface();
	return util->encodeTime(hours, minutes, seconds, fractions);
}

extern "C" ISC_DATE fbu_encode_date(void *master_ptr, unsigned year, unsigned month, unsigned day)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::IUtil* util = master->getUtilInterface();
	return util->encodeDate(year, month, day);
}

extern "C" void fbu_decode_time(void *master_ptr, ISC_TIME time, unsigned* hours, unsigned* minutes, unsigned* seconds, unsigned* fractions)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::IUtil* util = master->getUtilInterface();
	util->decodeTime(time, hours, minutes, seconds, fractions);
}

extern "C" void fbu_decode_date(void *master_ptr, ISC_DATE date, unsigned* year, unsigned* month, unsigned* day)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::IUtil* util = master->getUtilInterface();
	util->decodeDate(date, year, month, day);
}

#else

extern "C" ISC_TIME fbu_encode_time(void *master_ptr, unsigned hours, unsigned minutes, unsigned seconds, unsigned fractions)
{
	struct tm t = {};
	t.tm_hour = (int)hours;
	t.tm_min  = (int)minutes;
	t.tm_sec  = (int)seconds;
	ISC_TIME result;
	isc_encode_sql_time(&t, &result);
	return result;
}

extern "C" ISC_DATE fbu_encode_date(void *master_ptr, unsigned year, unsigned month, unsigned day)
{
	struct tm t = {};
	t.tm_year = (int)year - 1900;
	t.tm_mon  = (int)month - 1;
	t.tm_mday = (int)day;
	ISC_DATE result;
	isc_encode_sql_date(&t, &result);
	return result;
}

extern "C" void fbu_decode_time(void *master_ptr, ISC_TIME time, unsigned* hours, unsigned* minutes, unsigned* seconds, unsigned* fractions)
{
	struct tm t = {};
	isc_decode_sql_time(&time, &t);
	*hours    = (unsigned)t.tm_hour;
	*minutes  = (unsigned)t.tm_min;
	*seconds  = (unsigned)t.tm_sec;
	*fractions = 0;
}

extern "C" void fbu_decode_date(void *master_ptr, ISC_DATE date, unsigned* year, unsigned* month, unsigned* day)
{
	struct tm t = {};
	isc_decode_sql_date(&date, &t);
	*year  = (unsigned)(t.tm_year + 1900);
	*month = (unsigned)(t.tm_mon + 1);
	*day   = (unsigned)t.tm_mday;
}

#endif // FB_API_VER >= 30

#if FB_API_VER >= 40
static void fbu_copy_status(const ISC_STATUS* from, ISC_STATUS* to, size_t maxLength)
{
	for(size_t i=0; i < maxLength; ++i) {
		memcpy(to + i, from + i, sizeof(ISC_STATUS));
		if (from[i] == isc_arg_end) {
			break;
		}
	}
}

extern "C" void fbu_encode_time_tz(void *master_ptr, ISC_TIME_TZ* timeTz, unsigned hours, unsigned minutes, unsigned seconds, unsigned fractions, const char* timeZone)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::IUtil* util = master->getUtilInterface();
	Firebird::IStatus* status = master->getStatus();
	Firebird::CheckStatusWrapper st(status);
	util->encodeTimeTz(&st, timeTz, hours, minutes, seconds, fractions, timeZone);
}

extern "C" void fbu_encode_timestamp_tz(void *master_ptr, ISC_TIMESTAMP_TZ* timeStampTz, unsigned year, unsigned month, unsigned day, unsigned hours, unsigned minutes, unsigned seconds, unsigned fractions, const char* timeZone)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::IUtil* util = master->getUtilInterface();
	Firebird::IStatus* status = master->getStatus();
	Firebird::CheckStatusWrapper st(status);
	util->encodeTimeStampTz(&st, timeStampTz, year, month, day, hours, minutes, seconds, fractions, timeZone);
}

/* Decodes a time with time zone into its time components. */
extern "C" void fbu_decode_time_tz(void *master_ptr, const ISC_TIME_TZ* timeTz, unsigned* hours, unsigned* minutes, unsigned* seconds, unsigned* fractions,
   unsigned timeZoneBufferLength, char* timeZoneBuffer)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::IUtil* util = master->getUtilInterface();
	Firebird::IStatus* status = master->getStatus();
	Firebird::CheckStatusWrapper st(status);
	util->decodeTimeTz(&st, timeTz, hours, minutes, seconds, fractions,
						timeZoneBufferLength, timeZoneBuffer);
}

/* Decodes a timestamp with time zone into its date and time components */
extern "C" void fbu_decode_timestamp_tz(void *master_ptr, const ISC_TIMESTAMP_TZ* timestampTz,
	unsigned* year, unsigned* month, unsigned* day,
	unsigned* hours, unsigned* minutes, unsigned* seconds, unsigned* fractions,
	unsigned timeZoneBufferLength, char* timeZoneBuffer)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::IUtil* util = master->getUtilInterface();
	Firebird::IStatus* status = master->getStatus();
	Firebird::CheckStatusWrapper st(status);
	util->decodeTimeStampTz(&st, timestampTz, year, month, day,
							hours, minutes, seconds, fractions,
							timeZoneBufferLength, timeZoneBuffer);
}

extern "C" void fbu_release_statement(void *statement_ptr)
{
	Firebird::IStatement* statement = (Firebird::IStatement *)statement_ptr;
	if (statement) statement->release();
}

extern "C" int fbu_insert_aliases(void *master_ptr, ISC_STATUS* st, ibase_query *ib_query, void *statement_ptr)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::ThrowStatusWrapper status(master->getStatus());
	Firebird::IStatement* statement = (Firebird::IStatement *)statement_ptr;
	Firebird::IMessageMetadata* meta = NULL;
	ISC_STATUS res;

	try {
		meta = statement->getOutputMetadata(&status);
		unsigned cols = meta->getCount(&status);

		assert(cols == ib_query->out_fields_count);

		for (unsigned i = 0; i < cols; ++i)
		{
			_php_ibase_insert_alias(ib_query->ht_aliases, meta->getAlias(&status, i));
		}

		meta->release();
	}
	catch (const Firebird::FbException& error)
	{
		if (status.hasData())  {
			fbu_copy_status((const ISC_STATUS*)status.getErrors(), st, 20);
			return st[1];
		}
	}

	status.dispose();

	return 0;
}

extern "C" int fbu_insert_field_info(void *master_ptr, ISC_STATUS* st, int is_outvar, int num,
	zval *into_array, void *statement_ptr)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::ThrowStatusWrapper status(master->getStatus());
	Firebird::IStatement* statement = (Firebird::IStatement *)statement_ptr;
	Firebird::IMessageMetadata* meta = NULL;
	ISC_STATUS res;

	try {
		if(is_outvar) {
			meta = statement->getOutputMetadata(&status);
		} else {
			meta = statement->getInputMetadata(&status);
		}

		add_index_string(into_array, 0, meta->getField(&status, num));
		add_assoc_string(into_array, "name", meta->getField(&status, num));

		add_index_string(into_array, 1, meta->getAlias(&status, num));
		add_assoc_string(into_array, "alias", meta->getAlias(&status, num));

		add_index_string(into_array, 2, meta->getRelation(&status, num));
		add_assoc_string(into_array, "relation", meta->getRelation(&status, num));

		meta->release();
	}
	catch (const Firebird::FbException& error)
	{
		if (status.hasData())  {
			fbu_copy_status((const ISC_STATUS*)status.getErrors(), st, 20);
			return st[1];
		}
	}

	status.dispose();

	return 0;
}

#endif // FB_API_VER >= 40

#ifdef _MSC_VER
#include <intrin.h>
static int u64_mul10_gt(uint64_t r, uint64_t digit, uint64_t max, uint64_t *out)
{
	uint64_t hi, lo = _umul128(r, 10, &hi);
	if (hi || lo > max - digit) return 1;
	*out = lo + digit;
	return 0;
}
#else
static int u64_mul10_gt(uint64_t r, uint64_t digit, uint64_t max, uint64_t *out)
{
	unsigned __int128 v = (unsigned __int128)r * 10 + digit;
	if (v > max) return 1;
	*out = (uint64_t)v;
	return 0;
}
#endif

// Parse NUMERIC() to signed int representation
// Currently not used, bet keep it around. Useful when converting NUMERIC fields
// in integers. Might be usefult in the future.
extern "C" int fbu_string_to_numeric(const char *s, size_t slen, int scale, uint64_t max,
	int *sign, int *exp, uint64_t *res)
{
	const char* p = s;
	const char *end = s + slen;

	*sign = *exp = *res = 0;

	if (!slen) return STRNUM_PARSE_OK;

	if (*p == '-') {
		*sign = -1;
		p++;
	} else if (*p == '+') {
		*sign = 1;
		p++;
	} else {
		*sign = 1;
	}

	if (*sign == -1) max += 1;

	int fraction = 0;
	uint64_t r = *res;
	while (p < end) {
		if (*p >= '0' && *p <= '9') {
			if (u64_mul10_gt(r, *p - '0', max, &r)) return STRNUM_PARSE_OVERFLOW;
			p++;
			if (fraction) {
				scale++;
				--*exp;
			}
		} else if (*p == '.') {
			if (fraction) return STRNUM_PARSE_ERROR;
			fraction = 1;
			p++;
			continue;
		} else {
			return STRNUM_PARSE_ERROR;
		}
	}

	while (scale < 0) {
		if (u64_mul10_gt(r, 0, max, &r)) return STRNUM_PARSE_OVERFLOW;
		scale++;
		--*exp;
	}

	*res = r;

	return STRNUM_PARSE_OK;
}
