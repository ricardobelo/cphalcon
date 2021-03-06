
/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2013 Phalcon Team (http://www.phalconphp.com)       |
  +------------------------------------------------------------------------+
  | This source file is subject to the New BSD License that is bundled     |
  | with this package in the file docs/LICENSE.txt.                        |
  |                                                                        |
  | If you did not receive a copy of the license and are unable to         |
  | obtain it through the world-wide-web, please send an email             |
  | to license@phalconphp.com so we can send you a copy immediately.       |
  +------------------------------------------------------------------------+
  | Authors: Andres Gutierrez <andres@phalconphp.com>                      |
  |          Eduar Carvajal <eduar@phalconphp.com>                         |
  +------------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_phalcon.h"
#include "phalcon.h"

#include "Zend/zend_operators.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_interfaces.h"

#include "kernel/main.h"
#include "kernel/memory.h"

#include "kernel/object.h"
#include "kernel/concat.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"
#include "kernel/exception.h"
#include "kernel/string.h"
#include "kernel/array.h"

/**
 * Phalcon\Cache\Backend\Apc
 *
 * Allows to cache output fragments, PHP data and raw data using a memcache backend
 *
 *<code>
 *	//Cache data for 2 days
 *	$frontCache = new Phalcon\Cache\Frontend\Data(array(
 *		'lifetime' => 172800
 *	));
 *
 *  $cache = new Phalcon\Cache\Backend\Apc($frontCache, array(
 *      'prefix' => 'app-data'
 *  ));
 *
 *	//Cache arbitrary data
 *	$cache->save('my-data', array(1, 2, 3, 4, 5));
 *
 *	//Get data
 *	$data = $cache->get('my-data');
 *
 *</code>
 */


/**
 * Phalcon\Cache\Backend\Apc initializer
 */
PHALCON_INIT_CLASS(Phalcon_Cache_Backend_Apc){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Cache\\Backend, Apc, cache_backend_apc, "phalcon\\cache\\backend", phalcon_cache_backend_apc_method_entry, 0);

	zend_class_implements(phalcon_cache_backend_apc_ce TSRMLS_CC, 1, phalcon_cache_backendinterface_ce);

	return SUCCESS;
}

/**
 * Returns a cached content
 *
 * @param 	string $keyName
 * @param   long $lifetime
 * @return  mixed
 */
PHP_METHOD(Phalcon_Cache_Backend_Apc, get){

	zval *key_name, *lifetime = NULL, *frontend, *prefix, *prefixed_key;
	zval *cached_content, *processed;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z", &key_name, &lifetime) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!lifetime) {
		PHALCON_INIT_VAR(lifetime);
	}
	
	PHALCON_OBS_VAR(frontend);
	phalcon_read_property(&frontend, this_ptr, SL("_frontend"), PH_NOISY_CC);
	
	PHALCON_OBS_VAR(prefix);
	phalcon_read_property(&prefix, this_ptr, SL("_prefix"), PH_NOISY_CC);
	
	PHALCON_INIT_VAR(prefixed_key);
	PHALCON_CONCAT_SVV(prefixed_key, "_PHCA", prefix, key_name);
	phalcon_update_property_zval(this_ptr, SL("_lastKey"), prefixed_key TSRMLS_CC);
	
	PHALCON_INIT_VAR(cached_content);
	PHALCON_CALL_FUNC_PARAMS_1(cached_content, "apc_fetch", prefixed_key);
	if (PHALCON_IS_FALSE(cached_content)) {
		RETURN_MM_NULL();
	}
	
	PHALCON_INIT_VAR(processed);
	PHALCON_CALL_METHOD_PARAMS_1(processed, frontend, "afterretrieve", cached_content);
	
	RETURN_CCTOR(processed);
}

/**
 * Stores cached content into the APC backend and stops the frontend
 *
 * @param string $keyName
 * @param string $content
 * @param long $lifetime
 * @param boolean $stopBuffer
 */
PHP_METHOD(Phalcon_Cache_Backend_Apc, save){

	zval *key_name = NULL, *content = NULL, *lifetime = NULL, *stop_buffer = NULL;
	zval *last_key = NULL, *prefix, *frontend, *cached_content = NULL;
	zval *prepared_content, *ttl = NULL, *is_buffering;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zzzz", &key_name, &content, &lifetime, &stop_buffer) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!key_name) {
		PHALCON_INIT_VAR(key_name);
	}
	
	if (!content) {
		PHALCON_INIT_VAR(content);
	}
	
	if (!lifetime) {
		PHALCON_INIT_VAR(lifetime);
	}
	
	if (!stop_buffer) {
		PHALCON_INIT_VAR(stop_buffer);
		ZVAL_BOOL(stop_buffer, 1);
	}
	
	if (Z_TYPE_P(key_name) == IS_NULL) {
		PHALCON_OBS_VAR(last_key);
		phalcon_read_property(&last_key, this_ptr, SL("_lastKey"), PH_NOISY_CC);
	} else {
		PHALCON_OBS_VAR(prefix);
		phalcon_read_property(&prefix, this_ptr, SL("_prefix"), PH_NOISY_CC);
	
		PHALCON_INIT_NVAR(last_key);
		PHALCON_CONCAT_SVV(last_key, "_PHCA", prefix, key_name);
	}
	if (!zend_is_true(last_key)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "The cache must be started first");
		return;
	}
	
	PHALCON_OBS_VAR(frontend);
	phalcon_read_property(&frontend, this_ptr, SL("_frontend"), PH_NOISY_CC);
	if (Z_TYPE_P(content) == IS_NULL) {
		PHALCON_INIT_VAR(cached_content);
		PHALCON_CALL_METHOD(cached_content, frontend, "getcontent");
	} else {
		PHALCON_CPY_WRT(cached_content, content);
	}
	
	PHALCON_INIT_VAR(prepared_content);
	PHALCON_CALL_METHOD_PARAMS_1(prepared_content, frontend, "beforestore", cached_content);
	if (Z_TYPE_P(lifetime) == IS_NULL) {
		PHALCON_INIT_VAR(ttl);
		PHALCON_CALL_METHOD(ttl, frontend, "getlifetime");
	} else {
		PHALCON_CPY_WRT(ttl, lifetime);
	}
	
	PHALCON_CALL_FUNC_PARAMS_3_NORETURN("apc_store", last_key, prepared_content, ttl);
	
	PHALCON_INIT_VAR(is_buffering);
	PHALCON_CALL_METHOD(is_buffering, frontend, "isbuffering");
	if (PHALCON_IS_TRUE(stop_buffer)) {
		PHALCON_CALL_METHOD_NORETURN(frontend, "stop");
	}
	
	if (PHALCON_IS_TRUE(is_buffering)) {
		zend_print_zval(cached_content, 0);
	}
	
	phalcon_update_property_bool(this_ptr, SL("_started"), 0 TSRMLS_CC);
	
	PHALCON_MM_RESTORE();
}

/**
 * Deletes a value from the cache by its key
 *
 * @param string $keyName
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Apc, delete){

	zval *key_name, *prefix, *key, *success;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &key_name) == FAILURE) {
		RETURN_MM_NULL();
	}

	PHALCON_OBS_VAR(prefix);
	phalcon_read_property(&prefix, this_ptr, SL("_prefix"), PH_NOISY_CC);
	
	PHALCON_INIT_VAR(key);
	PHALCON_CONCAT_SVV(key, "_PHCA", prefix, key_name);
	
	PHALCON_INIT_VAR(success);
	PHALCON_CALL_FUNC_PARAMS_1(success, "apc_delete", key);
	RETURN_CCTOR(success);
}

/**
 * Query the existing cached keys
 *
 * @param string $prefix
 * @return array
 */
PHP_METHOD(Phalcon_Cache_Backend_Apc, queryKeys){

	zval *prefix = NULL, *keys, *type, *prefix_pattern, *iterator;
	zval *key = NULL, *real_key = NULL;
	zval *r0 = NULL;
	zend_class_entry *ce0;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &prefix) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!prefix) {
		PHALCON_INIT_VAR(prefix);
		ZVAL_STRING(prefix, "", 1);
	}
	
	PHALCON_INIT_VAR(keys);
	array_init(keys);
	
	PHALCON_INIT_VAR(type);
	ZVAL_STRING(type, "user", 1);
	
	PHALCON_INIT_VAR(prefix_pattern);
	PHALCON_CONCAT_SVS(prefix_pattern, "/^_PHCA", prefix, "/");
	ce0 = zend_fetch_class(SL("APCIterator"), ZEND_FETCH_CLASS_AUTO TSRMLS_CC);
	
	PHALCON_INIT_VAR(iterator);
	object_init_ex(iterator, ce0);
	if (phalcon_has_constructor(iterator TSRMLS_CC)) {
		PHALCON_CALL_METHOD_PARAMS_2_NORETURN(iterator, "__construct", type, prefix_pattern);
	}
	PHALCON_CALL_METHOD_NORETURN(iterator, "rewind");
	
	while (1) {
	
		PHALCON_INIT_NVAR(r0);
		PHALCON_CALL_METHOD(r0, iterator, "valid");
		if (zend_is_true(r0)) {
		} else {
			break;
		}
	
		PHALCON_INIT_NVAR(key);
		PHALCON_CALL_METHOD(key, iterator, "key");
	
		/** 
		 * Remove the _PHCA prefix
		 */
		PHALCON_INIT_NVAR(real_key);
		phalcon_substr(real_key, key, 5, 0 TSRMLS_CC);
		phalcon_array_append(&keys, real_key, PH_SEPARATE TSRMLS_CC);
		PHALCON_CALL_METHOD_NORETURN(iterator, "next");
	}
	
	RETURN_CTOR(keys);
}

/**
 * Checks if cache exists and it hasn't expired
 *
 * @param  string $keyName
 * @param  long $lifetime
 * @return boolean
 */
PHP_METHOD(Phalcon_Cache_Backend_Apc, exists){

	zval *key_name = NULL, *lifetime = NULL, *last_key = NULL, *prefix, *cache_exists;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zz", &key_name, &lifetime) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!key_name) {
		PHALCON_INIT_VAR(key_name);
	}
	
	if (!lifetime) {
		PHALCON_INIT_VAR(lifetime);
	}
	
	if (Z_TYPE_P(key_name) == IS_NULL) {
		PHALCON_OBS_VAR(last_key);
		phalcon_read_property(&last_key, this_ptr, SL("_lastKey"), PH_NOISY_CC);
	} else {
		PHALCON_OBS_VAR(prefix);
		phalcon_read_property(&prefix, this_ptr, SL("_prefix"), PH_NOISY_CC);
	
		PHALCON_INIT_NVAR(last_key);
		PHALCON_CONCAT_SVV(last_key, "_PHCA", prefix, key_name);
	}
	if (zend_is_true(last_key)) {
	
		PHALCON_INIT_VAR(cache_exists);
		PHALCON_CALL_FUNC_PARAMS_1(cache_exists, "apc_exists", last_key);
		if (PHALCON_IS_NOT_FALSE(cache_exists)) {
			RETURN_MM_TRUE;
		}
	}
	
	RETURN_MM_FALSE;
}

