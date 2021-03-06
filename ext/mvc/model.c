
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

#include "kernel/fcall.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/string.h"
#include "kernel/file.h"

/**
 * Phalcon\Mvc\Model
 *
 * <p>Phalcon\Mvc\Model connects business objects and database tables to create
 * a persistable domain model where logic and data are presented in one wrapping.
 * It‘s an implementation of the object-relational mapping (ORM).</p>
 *
 * <p>A model represents the information (data) of the application and the rules to manipulate that data.
 * Models are primarily used for managing the rules of interaction with a corresponding database table.
 * In most cases, each table in your database will correspond to one model in your application.
 * The bulk of your application’s business logic will be concentrated in the models.</p>
 *
 * <p>Phalcon\Mvc\Model is the first ORM written in C-language for PHP, giving to developers high performance
 * when interacting with databases while is also easy to use.</p>
 *
 * <code>
 *
 * $robot = new Robots();
 * $robot->type = 'mechanical'
 * $robot->name = 'Astro Boy';
 * $robot->year = 1952;
 * if ($robot->save() == false) {
 *  echo "Umh, We can store robots: ";
 *  foreach ($robot->getMessages() as $message) {
 *    echo $message;
 *  }
 * } else {
 *  echo "Great, a new robot was saved successfully!";
 * }
 * </code>
 *
 */


/**
 * Phalcon\Mvc\Model initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model){

	PHALCON_REGISTER_CLASS(Phalcon\\Mvc, Model, mvc_model, phalcon_mvc_model_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_null(phalcon_mvc_model_ce, SL("_dependencyInjector"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_model_ce, SL("_modelsManager"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_model_ce, SL("_modelsMetaData"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_model_ce, SL("_errorMessages"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_long(phalcon_mvc_model_ce, SL("_operationMade"), 0, ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_long(phalcon_mvc_model_ce, SL("_dirtyState"), 1, ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_model_ce, SL("_transaction"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_model_ce, SL("_uniqueKey"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_model_ce, SL("_uniqueParams"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_model_ce, SL("_uniqueTypes"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_model_ce, SL("_skipped"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_model_ce, SL("_related"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_model_ce, SL("_snapshot"), ZEND_ACC_PROTECTED TSRMLS_CC);

	zend_declare_class_constant_long(phalcon_mvc_model_ce, SL("OP_NONE"), 0 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_model_ce, SL("OP_CREATE"), 1 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_model_ce, SL("OP_UPDATE"), 2 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_model_ce, SL("OP_DELETE"), 3 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_model_ce, SL("DIRTY_STATE_PERSISTENT"), 0 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_model_ce, SL("DIRTY_STATE_TRANSIENT"), 1 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_model_ce, SL("DIRTY_STATE_DETACHED"), 2 TSRMLS_CC);

	zend_class_implements(phalcon_mvc_model_ce TSRMLS_CC, 4, phalcon_mvc_modelinterface_ce, phalcon_mvc_model_resultinterface_ce, phalcon_di_injectionawareinterface_ce, zend_ce_serializable);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\Model constructor
 *
 * @param Phalcon\DiInterface $dependencyInjector
 * @param Phalcon\Mvc\Model\ManagerInterface $modelsManager
 */
PHP_METHOD(Phalcon_Mvc_Model, __construct){

	zval *dependency_injector = NULL, *models_manager = NULL;
	zval *service_name;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zz", &dependency_injector, &models_manager) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!dependency_injector) {
		PHALCON_INIT_VAR(dependency_injector);
	} else {
		PHALCON_SEPARATE_PARAM(dependency_injector);
	}
	
	if (!models_manager) {
		PHALCON_INIT_VAR(models_manager);
	} else {
		PHALCON_SEPARATE_PARAM(models_manager);
	}
	
	/** 
	 * We use a default DI if the user doesn't define one
	 */
	if (Z_TYPE_P(dependency_injector) != IS_OBJECT) {
		PHALCON_INIT_NVAR(dependency_injector);
		PHALCON_CALL_STATIC(dependency_injector, "phalcon\\di", "getdefault");
	}
	if (Z_TYPE_P(dependency_injector) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "A dependency injector container is required to obtain the services related to the ORM");
		return;
	}
	
	phalcon_update_property_zval(this_ptr, SL("_dependencyInjector"), dependency_injector TSRMLS_CC);
	
	/** 
	 * Inject the manager service from the DI
	 */
	if (Z_TYPE_P(models_manager) != IS_OBJECT) {
	
		PHALCON_INIT_VAR(service_name);
		ZVAL_STRING(service_name, "modelsManager", 1);
	
		PHALCON_INIT_NVAR(models_manager);
		PHALCON_CALL_METHOD_PARAMS_1(models_manager, dependency_injector, "getshared", service_name);
		if (Z_TYPE_P(models_manager) != IS_OBJECT) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The injected service 'modelsManager' is not valid");
			return;
		}
	}
	
	/** 
	 * Update the models-manager
	 */
	phalcon_update_property_zval(this_ptr, SL("_modelsManager"), models_manager TSRMLS_CC);
	
	/** 
	 * The manager always initializes the object
	 */
	PHALCON_CALL_METHOD_PARAMS_1_NORETURN(models_manager, "initialize", this_ptr);
	
	PHALCON_MM_RESTORE();
}

/**
 * Sets the dependency injection container
 *
 * @param Phalcon\DiInterface $dependencyInjector
 */
PHP_METHOD(Phalcon_Mvc_Model, setDI){

	zval *dependency_injector;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &dependency_injector) == FAILURE) {
		RETURN_NULL();
	}

	phalcon_update_property_zval(this_ptr, SL("_dependencyInjector"), dependency_injector TSRMLS_CC);
	
}

/**
 * Returns the dependency injection container
 *
 * @return Phalcon\DiInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, getDI){


	RETURN_MEMBER(this_ptr, "_dependencyInjector");
}

/**
 * Sets a custom events manager
 *
 * @param Phalcon\Events\ManagerInterface $eventsManager
 */
PHP_METHOD(Phalcon_Mvc_Model, setEventsManager){

	zval *events_manager, *models_manager;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &events_manager) == FAILURE) {
		RETURN_MM_NULL();
	}

	PHALCON_OBS_VAR(models_manager);
	phalcon_read_property(&models_manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	PHALCON_CALL_METHOD_PARAMS_2_NORETURN(models_manager, "setcustomeventsmanager", this_ptr, events_manager);
	
	PHALCON_MM_RESTORE();
}

/**
 * Returns the custom events manager
 *
 * @return Phalcon\Events\ManagerInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, getEventsManager){

	zval *models_manager, *events_manager;

	PHALCON_MM_GROW();

	PHALCON_OBS_VAR(models_manager);
	phalcon_read_property(&models_manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	
	PHALCON_INIT_VAR(events_manager);
	PHALCON_CALL_METHOD_PARAMS_1(events_manager, models_manager, "getcustomeventsmanager", this_ptr);
	RETURN_CCTOR(events_manager);
}

/**
 * Returns the models meta-data service related to the entity instance
 *
 * @return Phalcon\Mvc\Model\MetaDataInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, getModelsMetaData){

	zval *meta_data = NULL, *dependency_injector, *service;

	PHALCON_MM_GROW();

	PHALCON_OBS_VAR(meta_data);
	phalcon_read_property(&meta_data, this_ptr, SL("_modelsMetaData"), PH_NOISY_CC);
	if (Z_TYPE_P(meta_data) != IS_OBJECT) {
	
		/** 
		 * Check if the DI is valid
		 */
		PHALCON_OBS_VAR(dependency_injector);
		phalcon_read_property(&dependency_injector, this_ptr, SL("_dependencyInjector"), PH_NOISY_CC);
		if (Z_TYPE_P(dependency_injector) != IS_OBJECT) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "A dependency injector container is required to obtain the services related to the ORM");
			return;
		}
	
		PHALCON_INIT_VAR(service);
		ZVAL_STRING(service, "modelsMetadata", 1);
	
		/** 
		 * Obtain the models-metadata service from the DI
		 */
		PHALCON_INIT_NVAR(meta_data);
		PHALCON_CALL_METHOD_PARAMS_1(meta_data, dependency_injector, "getshared", service);
		if (Z_TYPE_P(meta_data) != IS_OBJECT) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The injected service 'modelsMetadata' is not valid");
			return;
		}
	
		/** 
		 * Update the models-metada property
		 */
		phalcon_update_property_zval(this_ptr, SL("_modelsMetaData"), meta_data TSRMLS_CC);
	}
	
	
	RETURN_CCTOR(meta_data);
}

/**
 * Returns the models manager related to the entity instance
 *
 * @return Phalcon\Mvc\Model\ManagerInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, getModelsManager){


	RETURN_MEMBER(this_ptr, "_modelsManager");
}

/**
 * Sets a transaction related to the Model instance
 *
 *<code>
 *use Phalcon\Mvc\Model\Transaction\Manager as TxManager;
 *use Phalcon\Mvc\Model\Transaction\Failed as TxFailed;
 *
 *try {
 *
 *  $txManager = new TxManager();
 *
 *  $transaction = $txManager->get();
 *
 *  $robot = new Robots();
 *  $robot->setTransaction($transaction);
 *  $robot->name = 'WALL·E';
 *  $robot->created_at = date('Y-m-d');
 *  if ($robot->save() == false) {
 *    $transaction->rollback("Can't save robot");
 *  }
 *
 *  $robotPart = new RobotParts();
 *  $robotPart->setTransaction($transaction);
 *  $robotPart->type = 'head';
 *  if ($robotPart->save() == false) {
 *    $transaction->rollback("Robot part cannot be saved");
 *  }
 *
 *  $transaction->commit();
 *
 *} catch (TxFailed $e) {
 *  echo 'Failed, reason: ', $e->getMessage();
 *}
 *
 *</code>
 *
 * @param Phalcon\Mvc\Model\TransactionInterface $transaction
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, setTransaction){

	zval *transaction;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &transaction) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (Z_TYPE_P(transaction) == IS_OBJECT) {
		phalcon_update_property_zval(this_ptr, SL("_transaction"), transaction TSRMLS_CC);
		RETURN_THIS();
	}
	PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Transaction should be an object");
	return;
}

/**
 * Sets table name which model should be mapped
 *
 * @param string $source
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, setSource){

	zval *source, *models_manager;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &source) == FAILURE) {
		RETURN_MM_NULL();
	}

	PHALCON_OBS_VAR(models_manager);
	phalcon_read_property(&models_manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	PHALCON_CALL_METHOD_PARAMS_2_NORETURN(models_manager, "setmodelsource", this_ptr, source);
	RETURN_THIS();
}

/**
 * Returns table name mapped in the model
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model, getSource){

	zval *models_manager, *source;

	PHALCON_MM_GROW();

	PHALCON_OBS_VAR(models_manager);
	phalcon_read_property(&models_manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	
	PHALCON_INIT_VAR(source);
	PHALCON_CALL_METHOD_PARAMS_1(source, models_manager, "getmodelsource", this_ptr);
	RETURN_CCTOR(source);
}

/**
 * Sets schema name where table mapped is located
 *
 * @param string $schema
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, setSchema){

	zval *schema, *models_manager;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &schema) == FAILURE) {
		RETURN_MM_NULL();
	}

	PHALCON_OBS_VAR(models_manager);
	phalcon_read_property(&models_manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	PHALCON_CALL_METHOD_PARAMS_2_NORETURN(models_manager, "setmodelschema", this_ptr, schema);
	RETURN_THIS();
}

/**
 * Returns schema name where table mapped is located
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model, getSchema){

	zval *models_manager, *schema;

	PHALCON_MM_GROW();

	PHALCON_OBS_VAR(models_manager);
	phalcon_read_property(&models_manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	
	PHALCON_INIT_VAR(schema);
	PHALCON_CALL_METHOD_PARAMS_1(schema, models_manager, "getmodelschema", this_ptr);
	RETURN_CCTOR(schema);
}

/**
 * Sets the DependencyInjection connection service name
 *
 * @param string $connectionService
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, setConnectionService){

	zval *connection_service, *models_manager;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &connection_service) == FAILURE) {
		RETURN_MM_NULL();
	}

	PHALCON_OBS_VAR(models_manager);
	phalcon_read_property(&models_manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	PHALCON_CALL_METHOD_PARAMS_2_NORETURN(models_manager, "setconnectionservice", this_ptr, connection_service);
	RETURN_THIS();
}

/**
 * Sets the DependencyInjection connection service name used to read data
 *
 * @param string $connectionService
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, setReadConnectionService){

	zval *connection_service, *models_manager;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &connection_service) == FAILURE) {
		RETURN_MM_NULL();
	}

	PHALCON_OBS_VAR(models_manager);
	phalcon_read_property(&models_manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	PHALCON_CALL_METHOD_PARAMS_2_NORETURN(models_manager, "setreadconnectionservice", this_ptr, connection_service);
	RETURN_THIS();
}

/**
 * Sets the DependencyInjection connection service name used to write data
 *
 * @param string $connectionService
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, setWriteConnectionService){

	zval *connection_service, *models_manager;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &connection_service) == FAILURE) {
		RETURN_MM_NULL();
	}

	PHALCON_OBS_VAR(models_manager);
	phalcon_read_property(&models_manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	PHALCON_CALL_METHOD_PARAMS_2_NORETURN(models_manager, "setwriteconnectionservice", this_ptr, connection_service);
	RETURN_THIS();
}

/**
 * Returns the DependencyInjection connection service name used to read data related the model
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model, getReadConnectionService){

	zval *models_manager, *connection_service;

	PHALCON_MM_GROW();

	PHALCON_OBS_VAR(models_manager);
	phalcon_read_property(&models_manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	
	PHALCON_INIT_VAR(connection_service);
	PHALCON_CALL_METHOD_PARAMS_1(connection_service, models_manager, "getreadconnectionservice", this_ptr);
	RETURN_CCTOR(connection_service);
}

/**
 * Returns the DependencyInjection connection service name used to write data related to the model
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model, getWriteConnectionService){

	zval *models_manager, *connection_service;

	PHALCON_MM_GROW();

	PHALCON_OBS_VAR(models_manager);
	phalcon_read_property(&models_manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	
	PHALCON_INIT_VAR(connection_service);
	PHALCON_CALL_METHOD_PARAMS_1(connection_service, models_manager, "getwriteconnectionservice", this_ptr);
	RETURN_CCTOR(connection_service);
}

/**
 * Sets the dirty state of the object using one of the DIRTY_STATE_* constants
 *
 * @param int $dirtyState
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, setDirtyState){

	zval *dirty_state;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &dirty_state) == FAILURE) {
		RETURN_NULL();
	}

	phalcon_update_property_zval(this_ptr, SL("_dirtyState"), dirty_state TSRMLS_CC);
	RETURN_THISW();
}

/**
 * Returns one of the DIRTY_STATE_* constants telling if the record exists in the database or not
 *
 * @return int
 */
PHP_METHOD(Phalcon_Mvc_Model, getDirtyState){


	RETURN_MEMBER(this_ptr, "_dirtyState");
}

/**
 * Gets the connection used to read data for the model
 *
 * @return Phalcon\Db\AdapterInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, getReadConnection){

	zval *models_manager, *connection;

	PHALCON_MM_GROW();

	PHALCON_OBS_VAR(models_manager);
	phalcon_read_property(&models_manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	
	PHALCON_INIT_VAR(connection);
	PHALCON_CALL_METHOD_PARAMS_1(connection, models_manager, "getreadconnection", this_ptr);
	RETURN_CCTOR(connection);
}

/**
 * Gets the connection used to write data to the model
 *
 * @return Phalcon\Db\AdapterInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, getWriteConnection){

	zval *transaction, *connection = NULL, *models_manager;

	PHALCON_MM_GROW();

	PHALCON_OBS_VAR(transaction);
	phalcon_read_property(&transaction, this_ptr, SL("_transaction"), PH_NOISY_CC);
	if (Z_TYPE_P(transaction) == IS_OBJECT) {
		PHALCON_INIT_VAR(connection);
		PHALCON_CALL_METHOD(connection, transaction, "getconnection");
	} else {
		PHALCON_OBS_VAR(models_manager);
		phalcon_read_property(&models_manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	
		PHALCON_INIT_NVAR(connection);
		PHALCON_CALL_METHOD_PARAMS_1(connection, models_manager, "getwriteconnection", this_ptr);
	}
	
	
	RETURN_CCTOR(connection);
}

/**
 * Assigns values to a model from an array
 *
 *<code>
 *$robot->assign(array(
 *  'type' => 'mechanical',
 *  'name' => 'Astro Boy',
 *  'year' => 1952
 *));
 *</code>
 *
 * @param Phalcon\Mvc\Model $object
 * @param array $data
 * @param array $columnMap
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, assign){

	zval *data, *column_map = NULL, *value = NULL, *key = NULL, *attribute = NULL;
	zval *exception_message = NULL;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z", &data, &column_map) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!column_map) {
		PHALCON_INIT_VAR(column_map);
	}
	
	if (Z_TYPE_P(data) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Data to dump in the object must be an Array");
		return;
	}
	
	if (!phalcon_is_iterable(data, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
		return;
	}
	
	while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
		PHALCON_GET_FOREACH_KEY(key, ah0, hp0);
		PHALCON_GET_FOREACH_VALUE(value);
	
		/** 
		 * Only string keys in the data are valid
		 */
		if (Z_TYPE_P(column_map) == IS_ARRAY) { 
	
			/** 
			 * Every field must be part of the column map
			 */
			if (phalcon_array_isset(column_map, key)) {
				PHALCON_OBS_NVAR(attribute);
				phalcon_array_fetch(&attribute, column_map, key, PH_NOISY_CC);
				phalcon_update_property_zval_zval(this_ptr, attribute, value TSRMLS_CC);
			} else {
				PHALCON_INIT_NVAR(exception_message);
				PHALCON_CONCAT_SVS(exception_message, "Column \"", key, "\" doesn't make part of the column map");
				PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
				return;
			}
		} else {
			phalcon_update_property_zval_zval(this_ptr, key, value TSRMLS_CC);
		}
	
		zend_hash_move_forward_ex(ah0, &hp0);
	}
	
	
	PHALCON_MM_RESTORE();
}

/**
 * Assigns values to a model from an array returning a new model.
 *
 *<code>
 *$robot = \Phalcon\Mvc\Model::cloneResultMap(new Robots(), array(
 *  'type' => 'mechanical',
 *  'name' => 'Astro Boy',
 *  'year' => 1952
 *));
 *</code>
 *
 * @param Phalcon\Mvc\Model $base
 * @param array $data
 * @param array $columnMap
 * @param int $dirtyState
 * @param boolean $keepSnapshots
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, cloneResultMap){

	zval *base, *data, *column_map, *dirty_state = NULL, *keep_snapshots = NULL;
	zval *object, *value = NULL, *key = NULL, *attribute = NULL, *exception_message = NULL;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzz|zz", &base, &data, &column_map, &dirty_state, &keep_snapshots) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!dirty_state) {
		PHALCON_INIT_VAR(dirty_state);
		ZVAL_LONG(dirty_state, 0);
	}
	
	if (!keep_snapshots) {
		PHALCON_INIT_VAR(keep_snapshots);
	}
	
	if (Z_TYPE_P(data) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Data to dump in the object must be an Array");
		return;
	}
	
	PHALCON_INIT_VAR(object);
	if (phalcon_clone(object, base TSRMLS_CC) == FAILURE) {
		return;
	}
	
	/** 
	 * Change the dirty state to persistent
	 */
	PHALCON_CALL_METHOD_PARAMS_1_NORETURN(object, "setdirtystate", dirty_state);
	
	if (!phalcon_is_iterable(data, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
		return;
	}
	
	while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
		PHALCON_GET_FOREACH_KEY(key, ah0, hp0);
		PHALCON_GET_FOREACH_VALUE(value);
	
		if (Z_TYPE_P(key) == IS_STRING) {
	
			/** 
			 * Only string keys in the data are valid
			 */
			if (Z_TYPE_P(column_map) == IS_ARRAY) { 
	
				/** 
				 * Every field must be part of the column map
				 */
				if (phalcon_array_isset(column_map, key)) {
					PHALCON_OBS_NVAR(attribute);
					phalcon_array_fetch(&attribute, column_map, key, PH_NOISY_CC);
					phalcon_update_property_zval_zval(object, attribute, value TSRMLS_CC);
				} else {
					PHALCON_INIT_NVAR(exception_message);
					PHALCON_CONCAT_SVS(exception_message, "Column \"", key, "\" doesn't make part of the column map");
					PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
					return;
				}
			} else {
				phalcon_update_property_zval_zval(object, key, value TSRMLS_CC);
			}
		}
	
		zend_hash_move_forward_ex(ah0, &hp0);
	}
	
	if (zend_is_true(keep_snapshots)) {
		PHALCON_CALL_METHOD_PARAMS_2_NORETURN(object, "setsnapshotdata", data, column_map);
	}
	
	
	RETURN_CCTOR(object);
}

/**
 * Returns an hydrated result based on the data and the column map
 *
 * @param array $data
 * @param array $columnMap
 * @param int $hydrationMode
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Model, cloneResultMapHydrate){

	zval *data, *column_map, *hydration_mode, *hydrate = NULL;
	zval *value = NULL, *key = NULL, *exception_message = NULL, *attribute = NULL;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzz", &data, &column_map, &hydration_mode) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (Z_TYPE_P(data) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Data to hidrate must be an Array");
		return;
	}
	
	/** 
	 * If there is no column map and the hydration mode is arrays return the data as it
	 * is
	 */
	if (Z_TYPE_P(column_map) != IS_ARRAY) { 
		if (PHALCON_IS_LONG(hydration_mode, 1)) {
			RETURN_CCTOR(data);
		}
	}
	
	/** 
	 * Create the destination object according to the hydration mode
	 */
	if (PHALCON_IS_LONG(hydration_mode, 1)) {
		PHALCON_INIT_VAR(hydrate);
		array_init(hydrate);
	} else {
		PHALCON_INIT_NVAR(hydrate);
		object_init(hydrate);
	}
	
	
	if (!phalcon_is_iterable(data, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
		return;
	}
	
	while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
		PHALCON_GET_FOREACH_KEY(key, ah0, hp0);
		PHALCON_GET_FOREACH_VALUE(value);
	
		if (Z_TYPE_P(key) == IS_STRING) {
			if (Z_TYPE_P(column_map) == IS_ARRAY) { 
	
				/** 
				 * Every field must be part of the column map
				 */
				if (!phalcon_array_isset(column_map, key)) {
					PHALCON_INIT_NVAR(exception_message);
					PHALCON_CONCAT_SVS(exception_message, "Column \"", key, "\" doesn't make part of the column map");
					PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
					return;
				}
	
				PHALCON_OBS_NVAR(attribute);
				phalcon_array_fetch(&attribute, column_map, key, PH_NOISY_CC);
				if (PHALCON_IS_LONG(hydration_mode, 1)) {
					phalcon_array_update_zval(&hydrate, attribute, &value, PH_COPY | PH_SEPARATE TSRMLS_CC);
				} else {
					phalcon_update_property_zval_zval(hydrate, attribute, value TSRMLS_CC);
				}
			} else {
				if (PHALCON_IS_LONG(hydration_mode, 1)) {
					phalcon_array_update_zval(&hydrate, key, &value, PH_COPY | PH_SEPARATE TSRMLS_CC);
				} else {
					phalcon_update_property_zval_zval(hydrate, key, value TSRMLS_CC);
				}
			}
		}
	
		zend_hash_move_forward_ex(ah0, &hp0);
	}
	
	
	RETURN_CCTOR(hydrate);
}

/**
 * Assigns values to a model from an array returning a new model
 *
 *<code>
 *$robot = Phalcon\Mvc\Model::cloneResult(new Robots(), array(
 *  'type' => 'mechanical',
 *  'name' => 'Astro Boy',
 *  'year' => 1952
 *));
 *</code>
 *
 * @param Phalcon\Mvc\Model $base
 * @param array $data
 * @param int $dirtyState
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, cloneResult){

	zval *base, *data, *dirty_state = NULL, *object, *value = NULL, *key = NULL;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|z", &base, &data, &dirty_state) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!dirty_state) {
		PHALCON_INIT_VAR(dirty_state);
		ZVAL_LONG(dirty_state, 0);
	}
	
	if (Z_TYPE_P(data) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Data to dump in the object must be an Array");
		return;
	}
	
	PHALCON_INIT_VAR(object);
	if (phalcon_clone(object, base TSRMLS_CC) == FAILURE) {
		return;
	}
	PHALCON_CALL_METHOD_PARAMS_1_NORETURN(object, "setdirtystate", dirty_state);
	
	if (!phalcon_is_iterable(data, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
		return;
	}
	
	while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
		PHALCON_GET_FOREACH_KEY(key, ah0, hp0);
		PHALCON_GET_FOREACH_VALUE(value);
	
		if (Z_TYPE_P(key) != IS_STRING) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Invalid key in array data provided to dumpResult()");
			return;
		}
		phalcon_update_property_zval_zval(object, key, value TSRMLS_CC);
	
		zend_hash_move_forward_ex(ah0, &hp0);
	}
	
	
	RETURN_CCTOR(object);
}

/**
 * Allows to query a set of records that match the specified conditions
 *
 * <code>
 *
 * //How many robots are there?
 * $robots = Robots::find();
 * echo "There are ", count($robots), "\n";
 *
 * //How many mechanical robots are there?
 * $robots = Robots::find("type='mechanical'");
 * echo "There are ", count($robots), "\n";
 *
 * //Get and print virtual robots ordered by name
 * $robots = Robots::find(array("type='virtual'", "order" => "name"));
 * foreach ($robots as $robot) {
 *	   echo $robot->name, "\n";
 * }
 *
 * //Get first 100 virtual robots ordered by name
 * $robots = Robots::find(array("type='virtual'", "order" => "name", "limit" => 100));
 * foreach ($robots as $robot) {
 *	   echo $robot->name, "\n";
 * }
 * </code>
 *
 * @param 	array $parameters
 * @return  Phalcon\Mvc\Model\ResultsetInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, find){

	zval *parameters = NULL, *model_name, *params = NULL, *builder;
	zval *query, *bind_params = NULL, *bind_types = NULL, *cache;
	zval *resultset, *hydration;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &parameters) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!parameters) {
		PHALCON_INIT_VAR(parameters);
	}
	
	PHALCON_INIT_VAR(model_name);
	phalcon_get_called_class(model_name  TSRMLS_CC);
	if (Z_TYPE_P(parameters) != IS_ARRAY) { 
	
		PHALCON_INIT_VAR(params);
		array_init(params);
		if (Z_TYPE_P(parameters) != IS_NULL) {
			phalcon_array_append(&params, parameters, PH_SEPARATE TSRMLS_CC);
		}
	} else {
		PHALCON_CPY_WRT(params, parameters);
	}
	
	/** 
	 * Builds a query with the passed parameters
	 */
	PHALCON_INIT_VAR(builder);
	object_init_ex(builder, phalcon_mvc_model_query_builder_ce);
	PHALCON_CALL_METHOD_PARAMS_1_NORETURN(builder, "__construct", params);
	
	PHALCON_CALL_METHOD_PARAMS_1_NORETURN(builder, "from", model_name);
	
	PHALCON_INIT_VAR(query);
	PHALCON_CALL_METHOD(query, builder, "getquery");
	
	PHALCON_INIT_VAR(bind_params);
	
	PHALCON_INIT_VAR(bind_types);
	
	/** 
	 * Check for bind parameters
	 */
	if (phalcon_array_isset_string(params, SS("bind"))) {
	
		PHALCON_OBS_NVAR(bind_params);
		phalcon_array_fetch_string(&bind_params, params, SL("bind"), PH_NOISY_CC);
		if (phalcon_array_isset_string(params, SS("bindTypes"))) {
			PHALCON_OBS_NVAR(bind_types);
			phalcon_array_fetch_string(&bind_types, params, SL("bindTypes"), PH_NOISY_CC);
		}
	}
	
	/** 
	 * Pass the cache options to the query
	 */
	if (phalcon_array_isset_string(params, SS("cache"))) {
		PHALCON_OBS_VAR(cache);
		phalcon_array_fetch_string(&cache, params, SL("cache"), PH_NOISY_CC);
		PHALCON_CALL_METHOD_PARAMS_1_NORETURN(query, "cache", cache);
	}
	
	/** 
	 * Execute the query passing the bind-params and casting-types
	 */
	PHALCON_INIT_VAR(resultset);
	PHALCON_CALL_METHOD_PARAMS_2(resultset, query, "execute", bind_params, bind_types);
	
	/** 
	 * Define an hydration mode
	 */
	if (Z_TYPE_P(resultset) == IS_OBJECT) {
		if (phalcon_array_isset_string(params, SS("hydration"))) {
			PHALCON_OBS_VAR(hydration);
			phalcon_array_fetch_string(&hydration, params, SL("hydration"), PH_NOISY_CC);
			PHALCON_CALL_METHOD_PARAMS_1_NORETURN(resultset, "sethydratemode", hydration);
		}
	}
	
	
	RETURN_CCTOR(resultset);
}

/**
 * Allows to query the first record that match the specified conditions
 *
 * <code>
 *
 * //What's the first robot in robots table?
 * $robot = Robots::findFirst();
 * echo "The robot name is ", $robot->name;
 *
 * //What's the first mechanical robot in robots table?
 * $robot = Robots::findFirst("type='mechanical'");
 * echo "The first mechanical robot name is ", $robot->name;
 *
 * //Get first virtual robot ordered by name
 * $robot = Robots::findFirst(array("type='virtual'", "order" => "name"));
 * echo "The first virtual robot name is ", $robot->name;
 *
 * </code>
 *
 * @param array $parameters
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, findFirst){

	zval *parameters = NULL, *model_name, *params = NULL, *builder;
	zval *one, *query, *bind_params = NULL, *bind_types = NULL, *cache;
	zval *unique, *resultset;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &parameters) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!parameters) {
		PHALCON_INIT_VAR(parameters);
	}
	
	PHALCON_INIT_VAR(model_name);
	phalcon_get_called_class(model_name  TSRMLS_CC);
	if (Z_TYPE_P(parameters) != IS_ARRAY) { 
	
		PHALCON_INIT_VAR(params);
		array_init(params);
		if (Z_TYPE_P(parameters) != IS_NULL) {
			phalcon_array_append(&params, parameters, PH_SEPARATE TSRMLS_CC);
		}
	} else {
		PHALCON_CPY_WRT(params, parameters);
	}
	
	/** 
	 * Builds a query with the passed parameters
	 */
	PHALCON_INIT_VAR(builder);
	object_init_ex(builder, phalcon_mvc_model_query_builder_ce);
	PHALCON_CALL_METHOD_PARAMS_1_NORETURN(builder, "__construct", params);
	
	PHALCON_CALL_METHOD_PARAMS_1_NORETURN(builder, "from", model_name);
	
	/** 
	 * We only want the first record
	 */
	PHALCON_INIT_VAR(one);
	ZVAL_LONG(one, 1);
	PHALCON_CALL_METHOD_PARAMS_1_NORETURN(builder, "limit", one);
	
	PHALCON_INIT_VAR(query);
	PHALCON_CALL_METHOD(query, builder, "getquery");
	
	/** 
	 * Check for bind parameters
	 */
	PHALCON_INIT_VAR(bind_params);
	
	PHALCON_INIT_VAR(bind_types);
	if (phalcon_array_isset_string(params, SS("bind"))) {
	
		PHALCON_OBS_NVAR(bind_params);
		phalcon_array_fetch_string(&bind_params, params, SL("bind"), PH_NOISY_CC);
		if (phalcon_array_isset_string(params, SS("bindTypes"))) {
			PHALCON_OBS_NVAR(bind_types);
			phalcon_array_fetch_string(&bind_types, params, SL("bindTypes"), PH_NOISY_CC);
		}
	}
	
	/** 
	 * Pass the cache options to the query
	 */
	if (phalcon_array_isset_string(params, SS("cache"))) {
		PHALCON_OBS_VAR(cache);
		phalcon_array_fetch_string(&cache, params, SL("cache"), PH_NOISY_CC);
		PHALCON_CALL_METHOD_PARAMS_1_NORETURN(query, "cache", cache);
	}
	
	PHALCON_INIT_VAR(unique);
	ZVAL_BOOL(unique, 1);
	
	/** 
	 * Return only the first row
	 */
	PHALCON_CALL_METHOD_PARAMS_1_NORETURN(query, "setuniquerow", unique);
	
	/** 
	 * Execute the query passing the bind-params and casting-types
	 */
	PHALCON_INIT_VAR(resultset);
	PHALCON_CALL_METHOD_PARAMS_2(resultset, query, "execute", bind_params, bind_types);
	
	RETURN_CCTOR(resultset);
}

/**
 * Create a criteria for a specific model
 *
 * @param Phalcon\DiInterface $dependencyInjector
 * @return Phalcon\Mvc\Model\Criteria
 */
PHP_METHOD(Phalcon_Mvc_Model, query){

	zval *dependency_injector = NULL, *model_name, *criteria;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &dependency_injector) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!dependency_injector) {
		PHALCON_INIT_VAR(dependency_injector);
	} else {
		PHALCON_SEPARATE_PARAM(dependency_injector);
	}
	
	PHALCON_INIT_VAR(model_name);
	phalcon_get_called_class(model_name  TSRMLS_CC);
	
	/** 
	 * Use the global dependency injector if there is no one defined
	 */
	if (Z_TYPE_P(dependency_injector) != IS_OBJECT) {
		PHALCON_INIT_NVAR(dependency_injector);
		PHALCON_CALL_STATIC(dependency_injector, "phalcon\\di", "getdefault");
	}
	
	PHALCON_INIT_VAR(criteria);
	object_init_ex(criteria, phalcon_mvc_model_criteria_ce);
	PHALCON_CALL_METHOD_PARAMS_1_NORETURN(criteria, "setdi", dependency_injector);
	PHALCON_CALL_METHOD_PARAMS_1_NORETURN(criteria, "setmodelname", model_name);
	
	RETURN_CTOR(criteria);
}

/**
 * Checks if the current record already exists or not
 *
 * @param Phalcon\Mvc\Model\MetadataInterface $metaData
 * @param Phalcon\Db\AdapterInterface $connection
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, _exists){

	zval *meta_data, *connection, *table = NULL, *unique_params = NULL;
	zval *unique_types = NULL, *unique_key = NULL, *primary_keys;
	zval *bind_data_types, *number_primary, *column_map = NULL;
	zval *null_value, *number_empty, *where_pk, *field = NULL;
	zval *attribute_field = NULL, *exception_message = NULL;
	zval *value = NULL, *escaped_field = NULL, *pk_condition = NULL, *type = NULL;
	zval *join_where, *dirty_state, *schema, *source;
	zval *escaped_table, *null_mode, *select, *num;
	zval *row_count;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|z", &meta_data, &connection, &table) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!table) {
		PHALCON_INIT_VAR(table);
	} else {
		PHALCON_SEPARATE_PARAM(table);
	}
	
	PHALCON_INIT_VAR(unique_params);
	
	PHALCON_INIT_VAR(unique_types);
	
	/** 
	 * Builds a unique primary key condition
	 */
	PHALCON_OBS_VAR(unique_key);
	phalcon_read_property(&unique_key, this_ptr, SL("_uniqueKey"), PH_NOISY_CC);
	if (Z_TYPE_P(unique_key) == IS_NULL) {
	
		PHALCON_INIT_VAR(primary_keys);
		PHALCON_CALL_METHOD_PARAMS_1(primary_keys, meta_data, "getprimarykeyattributes", this_ptr);
	
		PHALCON_INIT_VAR(bind_data_types);
		PHALCON_CALL_METHOD_PARAMS_1(bind_data_types, meta_data, "getbindtypes", this_ptr);
	
		PHALCON_INIT_VAR(number_primary);
		phalcon_fast_count(number_primary, primary_keys TSRMLS_CC);
		if (!zend_is_true(number_primary)) {
			RETURN_MM_FALSE;
		}
	
		/** 
		 * Check if column renaming is globally activated
		 */
		if (PHALCON_GLOBAL(orm).column_renaming) {
			PHALCON_INIT_VAR(column_map);
			PHALCON_CALL_METHOD_PARAMS_1(column_map, meta_data, "getcolumnmap", this_ptr);
		} else {
			PHALCON_INIT_NVAR(column_map);
		}
	
		PHALCON_INIT_VAR(null_value);
	
		PHALCON_INIT_VAR(number_empty);
		ZVAL_LONG(number_empty, 0);
	
		PHALCON_INIT_VAR(where_pk);
		array_init(where_pk);
	
		array_init(unique_params);
	
		array_init(unique_types);
	
		/** 
		 * We need to create a primary key based on the current data
		 */
	
		if (!phalcon_is_iterable(primary_keys, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
			return;
		}
	
		while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
			PHALCON_GET_FOREACH_VALUE(field);
	
			if (Z_TYPE_P(column_map) == IS_ARRAY) { 
				if (phalcon_array_isset(column_map, field)) {
					PHALCON_OBS_NVAR(attribute_field);
					phalcon_array_fetch(&attribute_field, column_map, field, PH_NOISY_CC);
				} else {
					PHALCON_INIT_NVAR(exception_message);
					PHALCON_CONCAT_SVS(exception_message, "Column '", field, "\" isn't part of the column map");
					PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
					return;
				}
			} else {
				PHALCON_CPY_WRT(attribute_field, field);
			}
	
			/** 
			 * If the primary key attribute is set append it to the conditions
			 */
			if (phalcon_isset_property_zval(this_ptr, attribute_field TSRMLS_CC)) {
	
				PHALCON_OBS_NVAR(value);
				phalcon_read_property_zval(&value, this_ptr, attribute_field, PH_NOISY_CC);
	
				/** 
				 * We count how many fields are empty, if all fields are empy we don't perform an
				 * 'exist' check
				 */
				if (PHALCON_IS_EMPTY(value)) {
					PHALCON_SEPARATE(number_empty);
					increment_function(number_empty);
				}
	
				phalcon_array_append(&unique_params, value, PH_SEPARATE TSRMLS_CC);
			} else {
				phalcon_array_append(&unique_params, null_value, PH_SEPARATE TSRMLS_CC);
				PHALCON_SEPARATE(number_empty);
				increment_function(number_empty);
			}
	
			PHALCON_INIT_NVAR(escaped_field);
			PHALCON_CALL_METHOD_PARAMS_1(escaped_field, connection, "escapeidentifier", field);
	
			PHALCON_INIT_NVAR(pk_condition);
			PHALCON_CONCAT_VS(pk_condition, escaped_field, " = ?");
			if (!phalcon_array_isset(bind_data_types, field)) {
				PHALCON_INIT_NVAR(exception_message);
				PHALCON_CONCAT_SVS(exception_message, "Column '", field, "\" isn't part of the table columns");
				PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
				return;
			}
	
			PHALCON_OBS_NVAR(type);
			phalcon_array_fetch(&type, bind_data_types, field, PH_NOISY_CC);
			phalcon_array_append(&unique_types, type, PH_SEPARATE TSRMLS_CC);
			phalcon_array_append(&where_pk, pk_condition, PH_SEPARATE TSRMLS_CC);
	
			zend_hash_move_forward_ex(ah0, &hp0);
		}
	
		if (PHALCON_IS_EQUAL(number_primary, number_empty)) {
			RETURN_MM_FALSE;
		}
	
		PHALCON_INIT_VAR(join_where);
		phalcon_fast_join_str(join_where, SL(" AND "), where_pk TSRMLS_CC);
	
		/** 
		 * The unique key is composed of 3 parts _uniqueKey, uniqueParams, uniqueTypes
		 */
		phalcon_update_property_zval(this_ptr, SL("_uniqueKey"), join_where TSRMLS_CC);
		phalcon_update_property_zval(this_ptr, SL("_uniqueParams"), unique_params TSRMLS_CC);
		phalcon_update_property_zval(this_ptr, SL("_uniqueTypes"), unique_types TSRMLS_CC);
		PHALCON_CPY_WRT(unique_key, join_where);
	}
	
	/** 
	 * If we already know if the record exists we don't check it
	 */
	PHALCON_OBS_VAR(dirty_state);
	phalcon_read_property(&dirty_state, this_ptr, SL("_dirtyState"), PH_NOISY_CC);
	if (!zend_is_true(dirty_state)) {
		RETURN_MM_TRUE;
	}
	
	if (Z_TYPE_P(unique_key) == IS_NULL) {
		PHALCON_OBS_NVAR(unique_key);
		phalcon_read_property(&unique_key, this_ptr, SL("_uniqueKey"), PH_NOISY_CC);
	}
	
	if (Z_TYPE_P(unique_params) == IS_NULL) {
		PHALCON_OBS_NVAR(unique_params);
		phalcon_read_property(&unique_params, this_ptr, SL("_uniqueParams"), PH_NOISY_CC);
	}
	
	if (Z_TYPE_P(unique_types) == IS_NULL) {
		PHALCON_OBS_NVAR(unique_types);
		phalcon_read_property(&unique_types, this_ptr, SL("_uniqueTypes"), PH_NOISY_CC);
	}
	
	PHALCON_INIT_VAR(schema);
	PHALCON_CALL_METHOD(schema, this_ptr, "getschema");
	
	PHALCON_INIT_VAR(source);
	PHALCON_CALL_METHOD(source, this_ptr, "getsource");
	if (zend_is_true(schema)) {
		PHALCON_INIT_NVAR(table);
		array_init_size(table, 2);
		phalcon_array_append(&table, schema, PH_SEPARATE TSRMLS_CC);
		phalcon_array_append(&table, source, PH_SEPARATE TSRMLS_CC);
	} else {
		PHALCON_CPY_WRT(table, source);
	}
	
	PHALCON_INIT_VAR(escaped_table);
	PHALCON_CALL_METHOD_PARAMS_1(escaped_table, connection, "escapeidentifier", table);
	
	PHALCON_INIT_VAR(null_mode);
	
	/** 
	 * Here we use a single COUNT(*) without PHQL to make the execution faster
	 */
	PHALCON_INIT_VAR(select);
	PHALCON_CONCAT_SVSV(select, "SELECT COUNT(*) AS rowcount FROM ", escaped_table, " WHERE ", unique_key);
	
	PHALCON_INIT_VAR(num);
	PHALCON_CALL_METHOD_PARAMS_4(num, connection, "fetchone", select, null_mode, unique_params, unique_types);
	
	PHALCON_OBS_VAR(row_count);
	phalcon_array_fetch_string(&row_count, num, SL("rowcount"), PH_NOISY_CC);
	if (zend_is_true(row_count)) {
		phalcon_update_property_long(this_ptr, SL("_dirtyState"), 0 TSRMLS_CC);
		RETURN_MM_TRUE;
	} else {
		phalcon_update_property_long(this_ptr, SL("_dirtyState"), 1 TSRMLS_CC);
	}
	
	RETURN_MM_FALSE;
}

/**
 * Generate a PHQL SELECT statement for an aggregate
 *
 * @param string $function
 * @param string $alias
 * @param array $parameters
 * @return Phalcon\Mvc\Model\ResultsetInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, _groupResult){

	zval *function, *alias, *parameters, *params = NULL, *group_column = NULL;
	zval *distinct_column, *columns = NULL, *group_columns;
	zval *model_name, *builder, *query, *bind_params = NULL;
	zval *bind_types = NULL, *resultset, *cache, *number_rows;
	zval *first_row, *value;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzz", &function, &alias, &parameters) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (Z_TYPE_P(parameters) != IS_ARRAY) { 
		if (Z_TYPE_P(parameters) != IS_NULL) {
			PHALCON_INIT_VAR(params);
			array_init_size(params, 1);
			phalcon_array_append(&params, parameters, PH_SEPARATE TSRMLS_CC);
		} else {
			PHALCON_INIT_NVAR(params);
			array_init(params);
		}
	} else {
		PHALCON_CPY_WRT(params, parameters);
	}
	if (phalcon_array_isset_string(params, SS("column"))) {
		PHALCON_OBS_VAR(group_column);
		phalcon_array_fetch_string(&group_column, params, SL("column"), PH_NOISY_CC);
	} else {
		PHALCON_INIT_NVAR(group_column);
		ZVAL_STRING(group_column, "*", 1);
	}
	
	/** 
	 * Builds the columns to query according to the received parameters
	 */
	if (phalcon_array_isset_string(params, SS("distinct"))) {
		PHALCON_OBS_VAR(distinct_column);
		phalcon_array_fetch_string(&distinct_column, params, SL("distinct"), PH_NOISY_CC);
	
		PHALCON_INIT_VAR(columns);
		PHALCON_CONCAT_VSVSV(columns, function, "(DISTINCT ", distinct_column, ") AS ", alias);
	} else {
		if (phalcon_array_isset_string(params, SS("group"))) {
			PHALCON_OBS_VAR(group_columns);
			phalcon_array_fetch_string(&group_columns, params, SL("group"), PH_NOISY_CC);
	
			PHALCON_INIT_NVAR(columns);
			PHALCON_CONCAT_VSVSVSV(columns, group_columns, ", ", function, "(", group_column, ") AS ", alias);
		} else {
			PHALCON_INIT_NVAR(columns);
			PHALCON_CONCAT_VSVSV(columns, function, "(", group_column, ") AS ", alias);
		}
	}
	
	PHALCON_INIT_VAR(model_name);
	phalcon_get_called_class(model_name  TSRMLS_CC);
	
	/** 
	 * Builds a query with the passed parameters
	 */
	PHALCON_INIT_VAR(builder);
	object_init_ex(builder, phalcon_mvc_model_query_builder_ce);
	PHALCON_CALL_METHOD_PARAMS_1_NORETURN(builder, "__construct", params);
	
	PHALCON_CALL_METHOD_PARAMS_1_NORETURN(builder, "columns", columns);
	PHALCON_CALL_METHOD_PARAMS_1_NORETURN(builder, "from", model_name);
	
	PHALCON_INIT_VAR(query);
	PHALCON_CALL_METHOD(query, builder, "getquery");
	
	/** 
	 * Check for bind parameters
	 */
	PHALCON_INIT_VAR(bind_params);
	
	PHALCON_INIT_VAR(bind_types);
	if (phalcon_array_isset_string(params, SS("bind"))) {
	
		PHALCON_OBS_NVAR(bind_params);
		phalcon_array_fetch_string(&bind_params, params, SL("bind"), PH_NOISY_CC);
		if (phalcon_array_isset_string(params, SS("bindTypes"))) {
			PHALCON_OBS_NVAR(bind_types);
			phalcon_array_fetch_string(&bind_types, params, SL("bindTypes"), PH_NOISY_CC);
		}
	}
	
	/** 
	 * Execute the query
	 */
	PHALCON_INIT_VAR(resultset);
	PHALCON_CALL_METHOD_PARAMS_2(resultset, query, "execute", bind_params, bind_types);
	
	/** 
	 * Pass the cache options to the query
	 */
	if (phalcon_array_isset_string(params, SS("cache"))) {
		PHALCON_OBS_VAR(cache);
		phalcon_array_fetch_string(&cache, params, SL("cache"), PH_NOISY_CC);
		PHALCON_CALL_METHOD_PARAMS_1_NORETURN(query, "cache", cache);
	}
	
	/** 
	 * Return the full resultset if the query is grouped
	 */
	if (phalcon_array_isset_string(params, SS("group"))) {
		RETURN_CCTOR(resultset);
	}
	
	/** 
	 * Return only the value in the first result
	 */
	PHALCON_INIT_VAR(number_rows);
	phalcon_fast_count(number_rows, resultset TSRMLS_CC);
	
	PHALCON_INIT_VAR(first_row);
	PHALCON_CALL_METHOD(first_row, resultset, "getfirst");
	
	PHALCON_OBS_VAR(value);
	phalcon_read_property_zval(&value, first_row, alias, PH_NOISY_CC);
	
	RETURN_CCTOR(value);
}

/**
 * Allows to count how many records match the specified conditions
 *
 * <code>
 *
 * //How many robots are there?
 * $number = Robots::count();
 * echo "There are ", $number, "\n";
 *
 * //How many mechanical robots are there?
 * $number = Robots::count("type='mechanical'");
 * echo "There are ", $number, " mechanical robots\n";
 *
 * </code>
 *
 * @param array $parameters
 * @return int
 */
PHP_METHOD(Phalcon_Mvc_Model, count){

	zval *parameters = NULL, *function, *alias, *group;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &parameters) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!parameters) {
		PHALCON_INIT_VAR(parameters);
	}
	
	PHALCON_INIT_VAR(function);
	ZVAL_STRING(function, "COUNT", 1);
	
	PHALCON_INIT_VAR(alias);
	ZVAL_STRING(alias, "rowcount", 1);
	
	PHALCON_INIT_VAR(group);
	PHALCON_CALL_SELF_PARAMS_3(group, this_ptr, "_groupresult", function, alias, parameters);
	RETURN_CCTOR(group);
}

/**
 * Allows to calculate a summatory on a column that match the specified conditions
 *
 * <code>
 *
 * //How much are all robots?
 * $sum = Robots::sum(array('column' => 'price'));
 * echo "The total price of robots is ", $sum, "\n";
 *
 * //How much are mechanical robots?
 * $sum = Robots::sum(array("type='mechanical'", 'column' => 'price'));
 * echo "The total price of mechanical robots is  ", $sum, "\n";
 *
 * </code>
 *
 * @param array $parameters
 * @return double
 */
PHP_METHOD(Phalcon_Mvc_Model, sum){

	zval *parameters = NULL, *function, *alias, *group;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &parameters) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!parameters) {
		PHALCON_INIT_VAR(parameters);
	}
	
	PHALCON_INIT_VAR(function);
	ZVAL_STRING(function, "SUM", 1);
	
	PHALCON_INIT_VAR(alias);
	ZVAL_STRING(alias, "sumatory", 1);
	
	PHALCON_INIT_VAR(group);
	PHALCON_CALL_SELF_PARAMS_3(group, this_ptr, "_groupresult", function, alias, parameters);
	RETURN_CCTOR(group);
}

/**
 * Allows to get the maximum value of a column that match the specified conditions
 *
 * <code>
 *
 * //What is the maximum robot id?
 * $id = Robots::maximum(array('column' => 'id'));
 * echo "The maximum robot id is: ", $id, "\n";
 *
 * //What is the maximum id of mechanical robots?
 * $sum = Robots::maximum(array("type='mechanical'", 'column' => 'id'));
 * echo "The maximum robot id of mechanical robots is ", $id, "\n";
 *
 * </code>
 *
 * @param array $parameters
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Model, maximum){

	zval *parameters = NULL, *function, *alias, *group;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &parameters) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!parameters) {
		PHALCON_INIT_VAR(parameters);
	}
	
	PHALCON_INIT_VAR(function);
	ZVAL_STRING(function, "MAX", 1);
	
	PHALCON_INIT_VAR(alias);
	ZVAL_STRING(alias, "maximum", 1);
	
	PHALCON_INIT_VAR(group);
	PHALCON_CALL_SELF_PARAMS_3(group, this_ptr, "_groupresult", function, alias, parameters);
	RETURN_CCTOR(group);
}

/**
 * Allows to get the minimum value of a column that match the specified conditions
 *
 * <code>
 *
 * //What is the minimum robot id?
 * $id = Robots::minimum(array('column' => 'id'));
 * echo "The minimum robot id is: ", $id;
 *
 * //What is the minimum id of mechanical robots?
 * $sum = Robots::minimum(array("type='mechanical'", 'column' => 'id'));
 * echo "The minimum robot id of mechanical robots is ", $id;
 *
 * </code>
 *
 * @param array $parameters
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Model, minimum){

	zval *parameters = NULL, *function, *alias, *group;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &parameters) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!parameters) {
		PHALCON_INIT_VAR(parameters);
	}
	
	PHALCON_INIT_VAR(function);
	ZVAL_STRING(function, "MIN", 1);
	
	PHALCON_INIT_VAR(alias);
	ZVAL_STRING(alias, "minimum", 1);
	
	PHALCON_INIT_VAR(group);
	PHALCON_CALL_SELF_PARAMS_3(group, this_ptr, "_groupresult", function, alias, parameters);
	RETURN_CCTOR(group);
}

/**
 * Allows to calculate the average value on a column matching the specified conditions
 *
 * <code>
 *
 * //What's the average price of robots?
 * $average = Robots::average(array('column' => 'price'));
 * echo "The average price is ", $average, "\n";
 *
 * //What's the average price of mechanical robots?
 * $average = Robots::average(array("type='mechanical'", 'column' => 'price'));
 * echo "The average price of mechanical robots is ", $average, "\n";
 *
 * </code>
 *
 * @param array $parameters
 * @return double
 */
PHP_METHOD(Phalcon_Mvc_Model, average){

	zval *parameters = NULL, *function, *alias, *group;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &parameters) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!parameters) {
		PHALCON_INIT_VAR(parameters);
	}
	
	PHALCON_INIT_VAR(function);
	ZVAL_STRING(function, "AVG", 1);
	
	PHALCON_INIT_VAR(alias);
	ZVAL_STRING(alias, "average", 1);
	
	PHALCON_INIT_VAR(group);
	PHALCON_CALL_SELF_PARAMS_3(group, this_ptr, "_groupresult", function, alias, parameters);
	RETURN_CCTOR(group);
}

/**
 * Fires an event, implicitly calls behaviors and listeners in the events manager are notified
 *
 * @param string $eventName
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, fireEvent){

	zval *event_name, *models_manager, *success;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &event_name) == FAILURE) {
		RETURN_MM_NULL();
	}

	/** 
	 * Check if there is a method with the same name of the event
	 */
	if (phalcon_method_exists(this_ptr, event_name TSRMLS_CC) == SUCCESS) {
		PHALCON_CALL_METHOD_NORETURN(this_ptr, Z_STRVAL_P(event_name));
	}
	
	PHALCON_OBS_VAR(models_manager);
	phalcon_read_property(&models_manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	
	/** 
	 * Send a notification to the events manager
	 */
	PHALCON_INIT_VAR(success);
	PHALCON_CALL_METHOD_PARAMS_2(success, models_manager, "notifyevent", event_name, this_ptr);
	
	RETURN_CCTOR(success);
}

/**
 * Fires an event, implicitly calls behaviors and listeners in the events manager are notified
 * This method stops if one of the callbacks/listeners returns boolean false
 *
 * @param string $eventName
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, fireEventCancel){

	zval *event_name, *status = NULL, *models_manager;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &event_name) == FAILURE) {
		RETURN_MM_NULL();
	}

	/** 
	 * Check if there is a method with the same name of the event
	 */
	if (phalcon_method_exists(this_ptr, event_name TSRMLS_CC) == SUCCESS) {
	
		PHALCON_INIT_VAR(status);
		PHALCON_CALL_METHOD(status, this_ptr, Z_STRVAL_P(event_name));
		if (PHALCON_IS_FALSE(status)) {
			RETURN_MM_FALSE;
		}
	}
	
	PHALCON_OBS_VAR(models_manager);
	phalcon_read_property(&models_manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	
	/** 
	 * Send a notification to the events manager
	 */
	PHALCON_INIT_NVAR(status);
	PHALCON_CALL_METHOD_PARAMS_2(status, models_manager, "notifyevent", event_name, this_ptr);
	if (PHALCON_IS_FALSE(status)) {
		RETURN_MM_FALSE;
	}
	
	RETURN_MM_TRUE;
}

/**
 * Cancel the current operation
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, _cancelOperation){

	zval *operation_made, *event_name = NULL;

	PHALCON_MM_GROW();

	PHALCON_OBS_VAR(operation_made);
	phalcon_read_property(&operation_made, this_ptr, SL("_operationMade"), PH_NOISY_CC);
	if (PHALCON_IS_LONG(operation_made, 3)) {
		PHALCON_INIT_VAR(event_name);
		ZVAL_STRING(event_name, "notDeleted", 1);
	} else {
		PHALCON_INIT_NVAR(event_name);
		ZVAL_STRING(event_name, "notSaved", 1);
	}
	
	PHALCON_CALL_METHOD_PARAMS_1_NORETURN(this_ptr, "fireevent", event_name);
	
	PHALCON_MM_RESTORE();
}

/**
 * Appends a customized message on the validation process
 *
 * <code>
 * use \Phalcon\Mvc\Model\Message as Message;
 *
 * class Robots extends Phalcon\Mvc\Model
 * {
 *
 *   public function beforeSave()
 *   {
 *     if (this->name == 'Peter') {
 *        $message = new Message("Sorry, but a robot cannot be named Peter");
 *        $this->appendMessage($message);
 *     }
 *   }
 * }
 * </code>
 *
 * @param Phalcon\Mvc\Model\MessageInterface $message
 */
PHP_METHOD(Phalcon_Mvc_Model, appendMessage){

	zval *message, *type, *exception_message;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &message) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (Z_TYPE_P(message) != IS_OBJECT) {
		PHALCON_INIT_VAR(type);
		PHALCON_CALL_FUNC_PARAMS_1(type, "gettype", message);
	
		PHALCON_INIT_VAR(exception_message);
		PHALCON_CONCAT_SVS(exception_message, "Invalid message format '", type, "'");
		PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
		return;
	}
	phalcon_update_property_array_append(this_ptr, SL("_errorMessages"), message TSRMLS_CC);
	
	PHALCON_MM_RESTORE();
}

/**
 * Executes validators on every validation call
 *
 *<code>
 *use Phalcon\Mvc\Model\Validator\ExclusionIn as ExclusionIn;
 *
 *class Subscriptors extends Phalcon\Mvc\Model
 *{
 *
 *	public function validation()
 *  {
 * 		$this->validate(new ExclusionIn(array(
 *			'field' => 'status',
 *			'domain' => array('A', 'I')
 *		)));
 *		if ($this->validationHasFailed() == true) {
 *			return false;
 *		}
 *	}
 *
 *}
 *</code>
 *
 * @param object $validator
 */
PHP_METHOD(Phalcon_Mvc_Model, validate){

	zval *validator, *status, *messages, *message = NULL;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &validator) == FAILURE) {
		RETURN_MM_NULL();
	}

	/** 
	 * Valid validators are objects
	 */
	if (Z_TYPE_P(validator) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Validator must be an Object");
		return;
	}
	
	/** 
	 * Call the validation, if it returns false we append the messages to the current
	 * object
	 */
	PHALCON_INIT_VAR(status);
	PHALCON_CALL_METHOD_PARAMS_1(status, validator, "validate", this_ptr);
	if (PHALCON_IS_FALSE(status)) {
	
		PHALCON_INIT_VAR(messages);
		PHALCON_CALL_METHOD(messages, validator, "getmessages");
	
		if (!phalcon_is_iterable(messages, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
			return;
		}
	
		while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
			PHALCON_GET_FOREACH_VALUE(message);
	
			phalcon_update_property_array_append(this_ptr, SL("_errorMessages"), message TSRMLS_CC);
	
			zend_hash_move_forward_ex(ah0, &hp0);
		}
	
	}
	
	PHALCON_MM_RESTORE();
}

/**
 * Check whether validation process has generated any messages
 *
 *<code>
 *use Phalcon\Mvc\Model\Validator\ExclusionIn as ExclusionIn;
 *
 *class Subscriptors extends Phalcon\Mvc\Model
 *{
 *
 *	public function validation()
 *  {
 * 		$this->validate(new ExclusionIn(array(
 *			'field' => 'status',
 *			'domain' => array('A', 'I')
 *		)));
 *		if ($this->validationHasFailed() == true) {
 *			return false;
 *		}
 *	}
 *
 *}
 *</code>
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, validationHasFailed){

	zval *error_messages;

	PHALCON_MM_GROW();

	PHALCON_OBS_VAR(error_messages);
	phalcon_read_property(&error_messages, this_ptr, SL("_errorMessages"), PH_NOISY_CC);
	if (Z_TYPE_P(error_messages) == IS_ARRAY) { 
		if (phalcon_fast_count_ev(error_messages TSRMLS_CC)) {
			RETURN_MM_TRUE;
		}
	}
	
	RETURN_MM_FALSE;
}

/**
 * Returns all the validation messages
 *
 *<code>
 *	$robot = new Robots();
 *	$robot->type = 'mechanical';
 *	$robot->name = 'Astro Boy';
 *	$robot->year = 1952;
 *	if ($robot->save() == false) {
 *  	echo "Umh, We can't store robots right now ";
 *  	foreach ($robot->getMessages() as $message) {
 *			echo $message;
 *		}
 *	} else {
 *  	echo "Great, a new robot was saved successfully!";
 *	}
 * </code>
 *
 * @return Phalcon\Mvc\Model\MessageInterface[]
 */
PHP_METHOD(Phalcon_Mvc_Model, getMessages){


	RETURN_MEMBER(this_ptr, "_errorMessages");
}

/**
 * Reads "belongs to" relations and check the virtual foreign keys when inserting or updating records
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, _checkForeignKeys){

	zval *manager, *belongs_to, *relation = NULL, *foreign_key = NULL;
	zval *relation_class = NULL, *referenced_model = NULL, *conditions = NULL;
	zval *bind_params = NULL, *fields = NULL, *referenced_fields = NULL;
	zval *field = NULL, *position = NULL, *value = NULL, *referenced_field = NULL;
	zval *condition = NULL, *extra_conditions = NULL, *join_conditions = NULL;
	zval *parameters = NULL, *rowcount = NULL, *user_message = NULL, *joined_fields = NULL;
	zval *type = NULL, *message = NULL, *event_name;
	zend_bool error;
	HashTable *ah0, *ah1;
	HashPosition hp0, hp1;
	zval **hd;

	PHALCON_MM_GROW();

	/** 
	 * Get the models manager
	 */
	PHALCON_OBS_VAR(manager);
	phalcon_read_property(&manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	
	/** 
	 * We check if some of the belongsTo relations act as virtual foreign key
	 */
	PHALCON_INIT_VAR(belongs_to);
	PHALCON_CALL_METHOD_PARAMS_1(belongs_to, manager, "getbelongsto", this_ptr);
	if (phalcon_fast_count_ev(belongs_to TSRMLS_CC)) {
		error = 0;
	
		if (!phalcon_is_iterable(belongs_to, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
			return;
		}
	
		while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
			PHALCON_GET_FOREACH_VALUE(relation);
	
			PHALCON_INIT_NVAR(foreign_key);
			PHALCON_CALL_METHOD(foreign_key, relation, "getforeignkey");
			if (PHALCON_IS_NOT_FALSE(foreign_key)) {
	
				PHALCON_INIT_NVAR(relation_class);
				PHALCON_CALL_METHOD(relation_class, relation, "getreferencedmodel");
	
				/** 
				 * Load the referenced model if needed
				 */
				PHALCON_INIT_NVAR(referenced_model);
				PHALCON_CALL_METHOD_PARAMS_1(referenced_model, manager, "load", relation_class);
	
				/** 
				 * Since relations can have multiple columns or a single one, we need to build a
				 * condition for each of these cases
				 */
				PHALCON_INIT_NVAR(conditions);
				array_init(conditions);
	
				PHALCON_INIT_NVAR(bind_params);
				array_init(bind_params);
	
				PHALCON_INIT_NVAR(fields);
				PHALCON_CALL_METHOD(fields, relation, "getfields");
	
				PHALCON_INIT_NVAR(referenced_fields);
				PHALCON_CALL_METHOD(referenced_fields, relation, "getreferencedfields");
				if (Z_TYPE_P(fields) == IS_ARRAY) { 
	
					/** 
					 * Create a compound condition
					 */
	
					if (!phalcon_is_iterable(fields, &ah1, &hp1, 0, 0 TSRMLS_CC)) {
						return;
					}
	
					while (zend_hash_get_current_data_ex(ah1, (void**) &hd, &hp1) == SUCCESS) {
	
						PHALCON_GET_FOREACH_KEY(position, ah1, hp1);
						PHALCON_GET_FOREACH_VALUE(field);
	
						if (phalcon_isset_property_zval(this_ptr, field TSRMLS_CC)) {
							PHALCON_OBS_NVAR(value);
							phalcon_read_property_zval(&value, this_ptr, field, PH_NOISY_CC);
						} else {
							PHALCON_INIT_NVAR(value);
						}
	
						PHALCON_OBS_NVAR(referenced_field);
						phalcon_array_fetch(&referenced_field, referenced_fields, position, PH_NOISY_CC);
	
						PHALCON_INIT_NVAR(condition);
						PHALCON_CONCAT_VSV(condition, referenced_field, " = ?", position);
						phalcon_array_append(&conditions, condition, PH_SEPARATE TSRMLS_CC);
						phalcon_array_append(&bind_params, value, PH_SEPARATE TSRMLS_CC);
	
						zend_hash_move_forward_ex(ah1, &hp1);
					}
	
				} else {
					/** 
					 * Create a simple condition
					 */
					if (phalcon_isset_property_zval(this_ptr, fields TSRMLS_CC)) {
						PHALCON_OBS_NVAR(value);
						phalcon_read_property_zval(&value, this_ptr, fields, PH_NOISY_CC);
					} else {
						PHALCON_INIT_NVAR(value);
					}
	
					PHALCON_INIT_NVAR(condition);
					PHALCON_CONCAT_VS(condition, referenced_fields, " = ?0");
					phalcon_array_append(&conditions, condition, PH_SEPARATE TSRMLS_CC);
					phalcon_array_append(&bind_params, value, PH_SEPARATE TSRMLS_CC);
				}
	
				/** 
				 * Check if the virtual foreign key has extra conditions
				 */
				if (phalcon_array_isset_string(foreign_key, SS("conditions"))) {
					PHALCON_OBS_NVAR(extra_conditions);
					phalcon_array_fetch_string(&extra_conditions, foreign_key, SL("conditions"), PH_NOISY_CC);
					phalcon_array_append(&conditions, extra_conditions, PH_SEPARATE TSRMLS_CC);
				}
	
				/** 
				 * We don't trust the actual values in the object and pass the values using bound
				 * parameters
				 */
				PHALCON_INIT_NVAR(join_conditions);
				phalcon_fast_join_str(join_conditions, SL(" AND "), conditions TSRMLS_CC);
	
				PHALCON_INIT_NVAR(parameters);
				array_init_size(parameters, 2);
				phalcon_array_append(&parameters, join_conditions, PH_SEPARATE TSRMLS_CC);
				phalcon_array_update_string(&parameters, SL("bind"), &bind_params, PH_COPY | PH_SEPARATE TSRMLS_CC);
	
				/** 
				 * Let's make the checking
				 */
				PHALCON_INIT_NVAR(rowcount);
				PHALCON_CALL_METHOD_PARAMS_1(rowcount, referenced_model, "count", parameters);
				if (!zend_is_true(rowcount)) {
	
					/** 
					 * Get the user message or produce a new one
					 */
					if (phalcon_array_isset_string(foreign_key, SS("message"))) {
						PHALCON_OBS_NVAR(user_message);
						phalcon_array_fetch_string(&user_message, foreign_key, SL("message"), PH_NOISY_CC);
					} else {
						if (Z_TYPE_P(fields) == IS_ARRAY) { 
							PHALCON_INIT_NVAR(joined_fields);
							phalcon_fast_join_str(joined_fields, SL(", "), fields TSRMLS_CC);
	
							PHALCON_INIT_NVAR(user_message);
							PHALCON_CONCAT_SVS(user_message, "Value of fields \"", joined_fields, "\" does not exist on referenced table");
						} else {
							PHALCON_INIT_NVAR(user_message);
							PHALCON_CONCAT_SVS(user_message, "Value of field \"", fields, "\" does not exist on referenced table");
						}
					}
	
					/** 
					 * Create a message
					 */
					PHALCON_INIT_NVAR(type);
					ZVAL_STRING(type, "ConstraintViolation", 1);
	
					PHALCON_INIT_NVAR(message);
					object_init_ex(message, phalcon_mvc_model_message_ce);
					PHALCON_CALL_METHOD_PARAMS_3_NORETURN(message, "__construct", user_message, fields, type);
	
					PHALCON_CALL_METHOD_PARAMS_1_NORETURN(this_ptr, "appendmessage", message);
					error = 1;
					break;
				}
			}
	
			zend_hash_move_forward_ex(ah0, &hp0);
		}
	
		/** 
		 * Call 'onValidationFails' if the validation fails
		 */
		if (error) {
			if (PHALCON_GLOBAL(orm).events) {
				PHALCON_INIT_VAR(event_name);
				ZVAL_STRING(event_name, "onValidationFails", 1);
				PHALCON_CALL_METHOD_PARAMS_1_NORETURN(this_ptr, "fireevent", event_name);
				PHALCON_CALL_METHOD_NORETURN(this_ptr, "_canceloperation");
			}
			RETURN_MM_FALSE;
		}
	}
	
	RETURN_MM_TRUE;
}

/**
 * Reads both "hasMany" and "hasOne" relations and checks the virtual foreign keys when deleting records
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, _checkForeignKeysReverse){

	zval *manager, *relations, *error = NULL, *relation = NULL, *foreign_key = NULL;
	zval *relation_class = NULL, *referenced_model = NULL, *fields = NULL;
	zval *referenced_fields = NULL, *conditions = NULL, *bind_params = NULL;
	zval *field = NULL, *position = NULL, *value = NULL, *referenced_field = NULL;
	zval *condition = NULL, *extra_conditions = NULL, *join_conditions = NULL;
	zval *parameters = NULL, *rowcount = NULL, *user_message = NULL, *type = NULL;
	zval *message = NULL, *event_name;
	HashTable *ah0, *ah1;
	HashPosition hp0, hp1;
	zval **hd;

	PHALCON_MM_GROW();

	/** 
	 * Get the models manager
	 */
	PHALCON_OBS_VAR(manager);
	phalcon_read_property(&manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	
	/** 
	 * We check if some of the hasOne/hasMany relations is a foreign key
	 */
	PHALCON_INIT_VAR(relations);
	PHALCON_CALL_METHOD_PARAMS_1(relations, manager, "gethasoneandhasmany", this_ptr);
	if (phalcon_fast_count_ev(relations TSRMLS_CC)) {
	
		PHALCON_INIT_VAR(error);
		ZVAL_BOOL(error, 0);
	
		if (!phalcon_is_iterable(relations, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
			return;
		}
	
		while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
			PHALCON_GET_FOREACH_VALUE(relation);
	
			/** 
			 * Check if the relation has a virtual foreign key
			 */
			PHALCON_INIT_NVAR(foreign_key);
			PHALCON_CALL_METHOD(foreign_key, relation, "getforeignkey");
			if (PHALCON_IS_NOT_FALSE(foreign_key)) {
	
				PHALCON_INIT_NVAR(relation_class);
				PHALCON_CALL_METHOD(relation_class, relation, "getreferencedmodel");
	
				/** 
				 * Load a plain instance from the models manager
				 */
				PHALCON_INIT_NVAR(referenced_model);
				PHALCON_CALL_METHOD_PARAMS_1(referenced_model, manager, "load", relation_class);
	
				PHALCON_INIT_NVAR(fields);
				PHALCON_CALL_METHOD(fields, relation, "getfields");
	
				PHALCON_INIT_NVAR(referenced_fields);
				PHALCON_CALL_METHOD(referenced_fields, relation, "getreferencedfields");
	
				/** 
				 * Create the checking conditions. A relation can has many fields or a single one
				 */
				PHALCON_INIT_NVAR(conditions);
				array_init(conditions);
	
				PHALCON_INIT_NVAR(bind_params);
				array_init(bind_params);
				if (Z_TYPE_P(fields) == IS_ARRAY) { 
	
					if (!phalcon_is_iterable(fields, &ah1, &hp1, 0, 0 TSRMLS_CC)) {
						return;
					}
	
					while (zend_hash_get_current_data_ex(ah1, (void**) &hd, &hp1) == SUCCESS) {
	
						PHALCON_GET_FOREACH_KEY(position, ah1, hp1);
						PHALCON_GET_FOREACH_VALUE(field);
	
						if (phalcon_isset_property_zval(this_ptr, field TSRMLS_CC)) {
							PHALCON_OBS_NVAR(value);
							phalcon_read_property_zval(&value, this_ptr, field, PH_NOISY_CC);
						} else {
							PHALCON_INIT_NVAR(value);
						}
	
						PHALCON_OBS_NVAR(referenced_field);
						phalcon_array_fetch(&referenced_field, referenced_fields, position, PH_NOISY_CC);
	
						PHALCON_INIT_NVAR(condition);
						PHALCON_CONCAT_VSV(condition, referenced_field, " = ?", position);
						phalcon_array_append(&conditions, condition, PH_SEPARATE TSRMLS_CC);
						phalcon_array_append(&bind_params, value, PH_SEPARATE TSRMLS_CC);
	
						zend_hash_move_forward_ex(ah1, &hp1);
					}
	
				} else {
					if (phalcon_isset_property_zval(this_ptr, fields TSRMLS_CC)) {
						PHALCON_OBS_NVAR(value);
						phalcon_read_property_zval(&value, this_ptr, fields, PH_NOISY_CC);
					} else {
						PHALCON_INIT_NVAR(value);
					}
	
					PHALCON_INIT_NVAR(condition);
					PHALCON_CONCAT_VS(condition, referenced_fields, " = ?0");
					phalcon_array_append(&conditions, condition, PH_SEPARATE TSRMLS_CC);
					phalcon_array_append(&bind_params, value, PH_SEPARATE TSRMLS_CC);
				}
	
				/** 
				 * Check if the virtual foreign key has extra conditions
				 */
				if (phalcon_array_isset_string(foreign_key, SS("conditions"))) {
					PHALCON_OBS_NVAR(extra_conditions);
					phalcon_array_fetch_string(&extra_conditions, foreign_key, SL("conditions"), PH_NOISY_CC);
					phalcon_array_append(&conditions, extra_conditions, PH_SEPARATE TSRMLS_CC);
				}
	
				/** 
				 * We don't trust the actual values in the object and then we're passing the values
				 * using bound parameters
				 */
				PHALCON_INIT_NVAR(join_conditions);
				phalcon_fast_join_str(join_conditions, SL(" AND "), conditions TSRMLS_CC);
	
				PHALCON_INIT_NVAR(parameters);
				array_init_size(parameters, 2);
				phalcon_array_append(&parameters, join_conditions, PH_SEPARATE TSRMLS_CC);
				phalcon_array_update_string(&parameters, SL("bind"), &bind_params, PH_COPY | PH_SEPARATE TSRMLS_CC);
	
				/** 
				 * Let's make the checking
				 */
				PHALCON_INIT_NVAR(rowcount);
				PHALCON_CALL_METHOD_PARAMS_1(rowcount, referenced_model, "count", parameters);
				if (zend_is_true(rowcount)) {
	
					/** 
					 * Create a new message
					 */
					if (phalcon_array_isset_string(foreign_key, SS("message"))) {
						PHALCON_OBS_NVAR(user_message);
						phalcon_array_fetch_string(&user_message, foreign_key, SL("message"), PH_NOISY_CC);
					} else {
						PHALCON_INIT_NVAR(user_message);
						PHALCON_CONCAT_SV(user_message, "Record is referenced by model ", relation_class);
					}
	
					/** 
					 * Create a message
					 */
					PHALCON_INIT_NVAR(type);
					ZVAL_STRING(type, "ConstraintViolation", 1);
	
					PHALCON_INIT_NVAR(message);
					object_init_ex(message, phalcon_mvc_model_message_ce);
					PHALCON_CALL_METHOD_PARAMS_3_NORETURN(message, "__construct", user_message, fields, type);
	
					PHALCON_CALL_METHOD_PARAMS_1_NORETURN(this_ptr, "appendmessage", message);
	
					PHALCON_INIT_NVAR(error);
					ZVAL_BOOL(error, 1);
					break;
				}
			}
	
			zend_hash_move_forward_ex(ah0, &hp0);
		}
	
		/** 
		 * Call validation fails event
		 */
		if (PHALCON_IS_TRUE(error)) {
			if (PHALCON_GLOBAL(orm).events) {
				PHALCON_INIT_VAR(event_name);
				ZVAL_STRING(event_name, "onValidationFails", 1);
				PHALCON_CALL_METHOD_PARAMS_1_NORETURN(this_ptr, "fireevent", event_name);
				PHALCON_CALL_METHOD_NORETURN(this_ptr, "_canceloperation");
			}
			RETURN_MM_FALSE;
		}
	}
	
	RETURN_MM_TRUE;
}

/**
 * Executes internal hooks before save a record
 *
 * @param Phalcon\Mvc\Model\MetadataInterface $metaData
 * @param boolean $exists
 * @param string $identityField
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, _preSave){

	zval *meta_data, *exists, *identity_field, *event_name = NULL;
	zval *status = NULL, *not_null, *data_type_numeric;
	zval *column_map = NULL, *automatic_attributes = NULL, *error = NULL;
	zval *null_value, *field = NULL, *is_null = NULL, *attribute_field = NULL;
	zval *exception_message = NULL, *value = NULL, *message = NULL, *type = NULL;
	zval *model_message = NULL, *skipped;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzz", &meta_data, &exists, &identity_field) == FAILURE) {
		RETURN_MM_NULL();
	}

	/** 
	 * Run Validation Callbacks Before
	 */
	if (PHALCON_GLOBAL(orm).events) {
	
		PHALCON_INIT_VAR(event_name);
		ZVAL_STRING(event_name, "beforeValidation", 1);
	
		/** 
		 * Call the beforeValidation
		 */
		PHALCON_INIT_VAR(status);
		PHALCON_CALL_METHOD_PARAMS_1(status, this_ptr, "fireeventcancel", event_name);
		if (PHALCON_IS_FALSE(status)) {
			RETURN_MM_FALSE;
		}
	
		if (!zend_is_true(exists)) {
			PHALCON_INIT_NVAR(event_name);
			ZVAL_STRING(event_name, "beforeValidationOnCreate", 1);
		} else {
			PHALCON_INIT_NVAR(event_name);
			ZVAL_STRING(event_name, "beforeValidationOnUpdate", 1);
		}
	
		/** 
		 * Call the specific beforeValidation event for the current action
		 */
		PHALCON_INIT_NVAR(status);
		PHALCON_CALL_METHOD_PARAMS_1(status, this_ptr, "fireeventcancel", event_name);
		if (PHALCON_IS_FALSE(status)) {
			RETURN_MM_FALSE;
		}
	}
	
	/** 
	 * Check for Virtual foreign keys
	 */
	if (PHALCON_GLOBAL(orm).virtual_foreign_keys) {
	
		PHALCON_INIT_NVAR(status);
		PHALCON_CALL_METHOD(status, this_ptr, "_checkforeignkeys");
		if (PHALCON_IS_FALSE(status)) {
			RETURN_MM_FALSE;
		}
	}
	
	/** 
	 * Columns marked as not null are automatically validated by the ORM
	 */
	if (PHALCON_GLOBAL(orm).not_null_validations) {
	
		PHALCON_INIT_VAR(not_null);
		PHALCON_CALL_METHOD_PARAMS_1(not_null, meta_data, "getnotnullattributes", this_ptr);
		if (Z_TYPE_P(not_null) == IS_ARRAY) { 
	
			/** 
			 * Gets the fields that are numeric, these are validated in a diferent way
			 */
			PHALCON_INIT_VAR(data_type_numeric);
			PHALCON_CALL_METHOD_PARAMS_1(data_type_numeric, meta_data, "getdatatypesnumeric", this_ptr);
			if (PHALCON_GLOBAL(orm).column_renaming) {
				PHALCON_INIT_VAR(column_map);
				PHALCON_CALL_METHOD_PARAMS_1(column_map, meta_data, "getcolumnmap", this_ptr);
			} else {
				PHALCON_INIT_NVAR(column_map);
			}
	
			/** 
			 * Get fields that must be omitted from the SQL generation
			 */
			if (zend_is_true(exists)) {
				PHALCON_INIT_VAR(automatic_attributes);
				PHALCON_CALL_METHOD_PARAMS_1(automatic_attributes, meta_data, "getautomaticupdateattributes", this_ptr);
			} else {
				PHALCON_INIT_NVAR(automatic_attributes);
				PHALCON_CALL_METHOD_PARAMS_1(automatic_attributes, meta_data, "getautomaticcreateattributes", this_ptr);
			}
	
			PHALCON_INIT_VAR(error);
			ZVAL_BOOL(error, 0);
	
			PHALCON_INIT_VAR(null_value);
	
			if (!phalcon_is_iterable(not_null, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
				return;
			}
	
			while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
				PHALCON_GET_FOREACH_VALUE(field);
	
				/** 
				 * We don't check fields that must be omitted
				 */
				if (!phalcon_array_isset(automatic_attributes, field)) {
	
					PHALCON_INIT_NVAR(is_null);
					ZVAL_BOOL(is_null, 0);
					if (Z_TYPE_P(column_map) == IS_ARRAY) { 
						if (phalcon_array_isset(column_map, field)) {
							PHALCON_OBS_NVAR(attribute_field);
							phalcon_array_fetch(&attribute_field, column_map, field, PH_NOISY_CC);
						} else {
							PHALCON_INIT_NVAR(exception_message);
							PHALCON_CONCAT_SVS(exception_message, "Column '", field, "\" isn't part of the column map");
							PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
							return;
						}
					} else {
						PHALCON_CPY_WRT(attribute_field, field);
					}
	
					/** 
					 * Field is null when: 1) is not set, 2) is numeric but its value is not numeric,
					 * 3) is null or 4) is empty string
					 */
					if (phalcon_isset_property_zval(this_ptr, attribute_field TSRMLS_CC)) {
	
						PHALCON_OBS_NVAR(value);
						phalcon_read_property_zval(&value, this_ptr, attribute_field, PH_NOISY_CC);
						if (Z_TYPE_P(value) != IS_OBJECT) {
							if (!phalcon_array_isset(data_type_numeric, field)) {
								if (PHALCON_IS_EMPTY(value)) {
									PHALCON_INIT_NVAR(is_null);
									ZVAL_BOOL(is_null, 1);
								}
							} else {
								if (!phalcon_is_numeric(value)) {
									PHALCON_INIT_NVAR(is_null);
									ZVAL_BOOL(is_null, 1);
								}
							}
						}
					} else {
						PHALCON_INIT_NVAR(is_null);
						ZVAL_BOOL(is_null, 1);
					}
	
					if (PHALCON_IS_TRUE(is_null)) {
						if (!zend_is_true(exists)) {
	
							/** 
							 * The identity field can be null
							 */
							if (PHALCON_IS_EQUAL(field, identity_field)) {
								zend_hash_move_forward_ex(ah0, &hp0);
								continue;
							}
						}
	
						PHALCON_INIT_NVAR(message);
						PHALCON_CONCAT_VS(message, attribute_field, " is required");
	
						PHALCON_INIT_NVAR(type);
						ZVAL_STRING(type, "PresenceOf", 1);
	
						/** 
						 * A implicit PresenceOf message is created
						 */
						PHALCON_INIT_NVAR(model_message);
						object_init_ex(model_message, phalcon_mvc_model_message_ce);
						PHALCON_CALL_METHOD_PARAMS_3_NORETURN(model_message, "__construct", message, attribute_field, type);
	
						phalcon_update_property_array_append(this_ptr, SL("_errorMessages"), model_message TSRMLS_CC);
	
						PHALCON_INIT_NVAR(error);
						ZVAL_BOOL(error, 1);
					}
				}
	
				zend_hash_move_forward_ex(ah0, &hp0);
			}
	
			if (PHALCON_IS_TRUE(error)) {
				if (PHALCON_GLOBAL(orm).events) {
					PHALCON_INIT_NVAR(event_name);
					ZVAL_STRING(event_name, "onValidationFails", 1);
					PHALCON_CALL_METHOD_PARAMS_1_NORETURN(this_ptr, "fireevent", event_name);
					PHALCON_CALL_METHOD_NORETURN(this_ptr, "_canceloperation");
				}
				RETURN_MM_FALSE;
			}
		}
	}
	
	PHALCON_INIT_NVAR(event_name);
	ZVAL_STRING(event_name, "validation", 1);
	
	/** 
	 * Call the main validation event
	 */
	PHALCON_INIT_NVAR(status);
	PHALCON_CALL_METHOD_PARAMS_1(status, this_ptr, "fireeventcancel", event_name);
	if (PHALCON_IS_FALSE(status)) {
		if (PHALCON_GLOBAL(orm).events) {
			PHALCON_INIT_NVAR(event_name);
			ZVAL_STRING(event_name, "onValidationFails", 1);
			PHALCON_CALL_METHOD_PARAMS_1_NORETURN(this_ptr, "fireevent", event_name);
		}
		RETURN_MM_FALSE;
	}
	
	/** 
	 * Run Validation
	 */
	if (PHALCON_GLOBAL(orm).events) {
		if (!zend_is_true(exists)) {
			PHALCON_INIT_NVAR(event_name);
			ZVAL_STRING(event_name, "afterValidationOnCreate", 1);
		} else {
			PHALCON_INIT_NVAR(event_name);
			ZVAL_STRING(event_name, "afterValidationOnUpdate", 1);
		}
	
		/** 
		 * Run Validation Callbacks After
		 */
		PHALCON_INIT_NVAR(status);
		PHALCON_CALL_METHOD_PARAMS_1(status, this_ptr, "fireeventcancel", event_name);
		if (PHALCON_IS_FALSE(status)) {
			RETURN_MM_FALSE;
		}
	
		PHALCON_INIT_NVAR(event_name);
		ZVAL_STRING(event_name, "afterValidation", 1);
	
		PHALCON_INIT_NVAR(status);
		PHALCON_CALL_METHOD_PARAMS_1(status, this_ptr, "fireeventcancel", event_name);
		if (PHALCON_IS_FALSE(status)) {
			RETURN_MM_FALSE;
		}
	
		PHALCON_INIT_NVAR(event_name);
		ZVAL_STRING(event_name, "beforeSave", 1);
	
		/** 
		 * Run Before Callbacks
		 */
		PHALCON_INIT_NVAR(status);
		PHALCON_CALL_METHOD_PARAMS_1(status, this_ptr, "fireeventcancel", event_name);
		if (PHALCON_IS_FALSE(status)) {
			RETURN_MM_FALSE;
		}
	
		if (zend_is_true(exists)) {
			PHALCON_INIT_NVAR(event_name);
			ZVAL_STRING(event_name, "beforeUpdate", 1);
		} else {
			PHALCON_INIT_NVAR(event_name);
			ZVAL_STRING(event_name, "beforeCreate", 1);
		}
	
		phalcon_update_property_bool(this_ptr, SL("_skipped"), 0 TSRMLS_CC);
	
		/** 
		 * The operation can be skipped here
		 */
		PHALCON_INIT_NVAR(status);
		PHALCON_CALL_METHOD_PARAMS_1(status, this_ptr, "fireeventcancel", event_name);
		if (PHALCON_IS_FALSE(status)) {
			RETURN_MM_FALSE;
		}
	
		/** 
		 * Always return true if the operation is skipped
		 */
		PHALCON_OBS_VAR(skipped);
		phalcon_read_property(&skipped, this_ptr, SL("_skipped"), PH_NOISY_CC);
		if (PHALCON_IS_TRUE(skipped)) {
			RETURN_MM_TRUE;
		}
	}
	
	RETURN_MM_TRUE;
}

/**
 * Executes internal events after save a record
 *
 * @param boolean $success
 * @param boolean $exists
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, _postSave){

	zval *success, *exists, *event_name = NULL;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &success, &exists) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (PHALCON_IS_TRUE(success)) {
		if (zend_is_true(exists)) {
			PHALCON_INIT_VAR(event_name);
			ZVAL_STRING(event_name, "afterUpdate", 1);
		} else {
			PHALCON_INIT_NVAR(event_name);
			ZVAL_STRING(event_name, "afterCreate", 1);
		}
		PHALCON_CALL_METHOD_PARAMS_1_NORETURN(this_ptr, "fireevent", event_name);
	
		PHALCON_INIT_NVAR(event_name);
		ZVAL_STRING(event_name, "afterSave", 1);
		PHALCON_CALL_METHOD_PARAMS_1_NORETURN(this_ptr, "fireevent", event_name);
	
		RETURN_CCTOR(success);
	}
	
	PHALCON_INIT_NVAR(event_name);
	ZVAL_STRING(event_name, "notSave", 1);
	PHALCON_CALL_METHOD_PARAMS_1_NORETURN(this_ptr, "fireevent", event_name);
	PHALCON_CALL_METHOD_NORETURN(this_ptr, "_canceloperation");
	RETURN_MM_FALSE;
}

/**
 * Sends a pre-build INSERT SQL statement to the relational database system
 *
 * @param Phalcon\Mvc\Model\MetadataInterface $metaData
 * @param Phalcon\Db\AdapterInterface $connection
 * @param string $table
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, _doLowInsert){

	zval *meta_data, *connection, *table, *identity_field;
	zval *null_value, *bind_skip, *fields, *values;
	zval *bind_types, *attributes, *bind_data_types;
	zval *automatic_attributes, *column_map = NULL, *field = NULL;
	zval *attribute_field = NULL, *exception_message = NULL;
	zval *value = NULL, *bind_type = NULL, *default_value, *success;
	zval *sequence_name = NULL, *support_sequences, *source;
	zval *last_insert_id;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzzz", &meta_data, &connection, &table, &identity_field) == FAILURE) {
		RETURN_MM_NULL();
	}

	PHALCON_INIT_VAR(null_value);
	
	PHALCON_INIT_VAR(bind_skip);
	ZVAL_LONG(bind_skip, 1024);
	
	PHALCON_INIT_VAR(fields);
	array_init(fields);
	
	PHALCON_INIT_VAR(values);
	array_init(values);
	
	PHALCON_INIT_VAR(bind_types);
	array_init(bind_types);
	
	PHALCON_INIT_VAR(attributes);
	PHALCON_CALL_METHOD_PARAMS_1(attributes, meta_data, "getattributes", this_ptr);
	
	PHALCON_INIT_VAR(bind_data_types);
	PHALCON_CALL_METHOD_PARAMS_1(bind_data_types, meta_data, "getbindtypes", this_ptr);
	
	PHALCON_INIT_VAR(automatic_attributes);
	PHALCON_CALL_METHOD_PARAMS_1(automatic_attributes, meta_data, "getautomaticcreateattributes", this_ptr);
	if (PHALCON_GLOBAL(orm).column_renaming) {
		PHALCON_INIT_VAR(column_map);
		PHALCON_CALL_METHOD_PARAMS_1(column_map, meta_data, "getcolumnmap", this_ptr);
	} else {
		PHALCON_INIT_NVAR(column_map);
	}
	
	/** 
	 * All fields in the model makes part or the INSERT
	 */
	
	if (!phalcon_is_iterable(attributes, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
		return;
	}
	
	while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
		PHALCON_GET_FOREACH_VALUE(field);
	
		if (!phalcon_array_isset(automatic_attributes, field)) {
	
			/** 
			 * Check if the model has a column map
			 */
			if (Z_TYPE_P(column_map) == IS_ARRAY) { 
				if (phalcon_array_isset(column_map, field)) {
					PHALCON_OBS_NVAR(attribute_field);
					phalcon_array_fetch(&attribute_field, column_map, field, PH_NOISY_CC);
				} else {
					PHALCON_INIT_NVAR(exception_message);
					PHALCON_CONCAT_SVS(exception_message, "Column '", field, "\" isn't part of the column map");
					PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
					return;
				}
			} else {
				PHALCON_CPY_WRT(attribute_field, field);
			}
	
			/** 
			 * Check every attribute in the model except identity field
			 */
			if (!PHALCON_IS_EQUAL(field, identity_field)) {
				phalcon_array_append(&fields, field, PH_SEPARATE TSRMLS_CC);
				if (phalcon_isset_property_zval(this_ptr, attribute_field TSRMLS_CC)) {
					if (!phalcon_array_isset(bind_data_types, field)) {
						PHALCON_INIT_NVAR(exception_message);
						PHALCON_CONCAT_SVS(exception_message, "Column '", field, "\" isn't part of the table columns");
						PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
						return;
					}
	
					PHALCON_OBS_NVAR(value);
					phalcon_read_property_zval(&value, this_ptr, attribute_field, PH_NOISY_CC);
					phalcon_array_append(&values, value, PH_SEPARATE TSRMLS_CC);
	
					PHALCON_OBS_NVAR(bind_type);
					phalcon_array_fetch(&bind_type, bind_data_types, field, PH_NOISY_CC);
					phalcon_array_append(&bind_types, bind_type, PH_SEPARATE TSRMLS_CC);
				} else {
					phalcon_array_append(&values, null_value, PH_SEPARATE TSRMLS_CC);
					phalcon_array_append(&bind_types, bind_skip, PH_SEPARATE TSRMLS_CC);
				}
			}
		}
	
		zend_hash_move_forward_ex(ah0, &hp0);
	}
	
	/** 
	 * If there is an identity field we add it using "null" or "default"
	 */
	if (PHALCON_IS_NOT_FALSE(identity_field)) {
	
		PHALCON_INIT_VAR(default_value);
		PHALCON_CALL_METHOD(default_value, connection, "getdefaultidvalue");
		phalcon_array_append(&fields, identity_field, PH_SEPARATE TSRMLS_CC);
	
		/** 
		 * Check if the model has a column map
		 */
		if (Z_TYPE_P(column_map) == IS_ARRAY) { 
			if (phalcon_array_isset(column_map, identity_field)) {
				PHALCON_OBS_NVAR(attribute_field);
				phalcon_array_fetch(&attribute_field, column_map, identity_field, PH_NOISY_CC);
			} else {
				PHALCON_INIT_NVAR(exception_message);
				PHALCON_CONCAT_SVS(exception_message, "Column '", identity_field, "\" isn't part of the column map");
				PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
				return;
			}
		} else {
			PHALCON_CPY_WRT(attribute_field, identity_field);
		}
	
		if (phalcon_isset_property_zval(this_ptr, attribute_field TSRMLS_CC)) {
	
			PHALCON_OBS_NVAR(value);
			phalcon_read_property_zval(&value, this_ptr, attribute_field, PH_NOISY_CC);
			if (PHALCON_IS_EMPTY(value)) {
				phalcon_array_append(&values, default_value, PH_SEPARATE TSRMLS_CC);
				phalcon_array_append(&bind_data_types, bind_skip, PH_SEPARATE TSRMLS_CC);
			} else {
				/** 
				 * The field is valid we look for a bind value (normally int)
				 */
				if (!phalcon_array_isset(bind_data_types, identity_field)) {
					PHALCON_INIT_NVAR(exception_message);
					PHALCON_CONCAT_SVS(exception_message, "Identity column '", identity_field, "\" isn't part of the table columns");
					PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
					return;
				}
				phalcon_array_append(&values, value, PH_SEPARATE TSRMLS_CC);
	
				PHALCON_OBS_NVAR(bind_type);
				phalcon_array_fetch(&bind_type, bind_data_types, identity_field, PH_NOISY_CC);
				phalcon_array_append(&bind_types, bind_type, PH_SEPARATE TSRMLS_CC);
			}
		} else {
			phalcon_array_append(&values, default_value, PH_SEPARATE TSRMLS_CC);
			phalcon_array_append(&bind_types, bind_skip, PH_SEPARATE TSRMLS_CC);
		}
	}
	
	/** 
	 * The low level insert is performed
	 */
	PHALCON_INIT_VAR(success);
	PHALCON_CALL_METHOD_PARAMS_4(success, connection, "insert", table, values, fields, bind_types);
	if (PHALCON_IS_NOT_FALSE(identity_field)) {
	
		/** 
		 * We check if the model have sequences
		 */
		PHALCON_INIT_VAR(sequence_name);
	
		PHALCON_INIT_VAR(support_sequences);
		PHALCON_CALL_METHOD(support_sequences, connection, "supportsequences");
		if (PHALCON_IS_TRUE(support_sequences)) {
			if (phalcon_method_exists_ex(this_ptr, SS("getsequencename") TSRMLS_CC) == SUCCESS) {
				PHALCON_CALL_METHOD(sequence_name, this_ptr, "getsequencename");
			} else {
				PHALCON_INIT_VAR(source);
				PHALCON_CALL_METHOD(source, this_ptr, "getsource");
	
				PHALCON_INIT_NVAR(sequence_name);
				PHALCON_CONCAT_VSVS(sequence_name, source, "_", identity_field, "_seq");
			}
		}
	
		/** 
		 * Recover the last "insert id" and assign it to the object
		 */
		PHALCON_INIT_VAR(last_insert_id);
		PHALCON_CALL_METHOD_PARAMS_1(last_insert_id, connection, "lastinsertid", sequence_name);
		phalcon_update_property_zval_zval(this_ptr, attribute_field, last_insert_id TSRMLS_CC);
	
		/** 
		 * Since the primary key was modified, we delete the _uniqueParams to force any
		 * future update to re-build the primary key
		 */
		phalcon_update_property_null(this_ptr, SL("_uniqueParams") TSRMLS_CC);
	}
	
	
	RETURN_CCTOR(success);
}

/**
 * Sends a pre-build UPDATE SQL statement to the relational database system
 *
 * @param Phalcon\Mvc\Model\MetadataInterface $metaData
 * @param Phalcon\Db\AdapterInterface $connection
 * @param string|array $table
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, _doLowUpdate){

	zval *meta_data, *connection, *table, *null_value;
	zval *bind_skip, *fields, *values, *bind_types;
	zval *manager, *use_dynamic_update = NULL, *snapshot;
	zval *bind_data_types, *non_primary, *automatic_attributes;
	zval *column_map = NULL, *field = NULL, *exception_message = NULL;
	zval *attribute_field = NULL, *value = NULL, *bind_type = NULL, *changed = NULL;
	zval *snapshot_value = NULL, *unique_key, *unique_params = NULL;
	zval *unique_types, *primary_keys, *conditions;
	zval *success;
	HashTable *ah0, *ah1;
	HashPosition hp0, hp1;
	zval **hd;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzz", &meta_data, &connection, &table) == FAILURE) {
		RETURN_MM_NULL();
	}

	PHALCON_INIT_VAR(null_value);
	
	PHALCON_INIT_VAR(bind_skip);
	ZVAL_LONG(bind_skip, 1024);
	
	PHALCON_INIT_VAR(fields);
	array_init(fields);
	
	PHALCON_INIT_VAR(values);
	array_init(values);
	
	PHALCON_INIT_VAR(bind_types);
	array_init(bind_types);
	
	PHALCON_OBS_VAR(manager);
	phalcon_read_property(&manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	
	/** 
	 * Check if the model must use dynamic update
	 */
	PHALCON_INIT_VAR(use_dynamic_update);
	PHALCON_CALL_METHOD_PARAMS_1(use_dynamic_update, manager, "isusingdynamicupdate", this_ptr);
	if (zend_is_true(use_dynamic_update)) {
	
		PHALCON_OBS_VAR(snapshot);
		phalcon_read_property(&snapshot, this_ptr, SL("_snapshot"), PH_NOISY_CC);
		if (Z_TYPE_P(snapshot) != IS_ARRAY) { 
			PHALCON_INIT_NVAR(use_dynamic_update);
			ZVAL_BOOL(use_dynamic_update, 0);
		}
	}
	
	PHALCON_INIT_VAR(bind_data_types);
	PHALCON_CALL_METHOD_PARAMS_1(bind_data_types, meta_data, "getbindtypes", this_ptr);
	
	PHALCON_INIT_VAR(non_primary);
	PHALCON_CALL_METHOD_PARAMS_1(non_primary, meta_data, "getnonprimarykeyattributes", this_ptr);
	
	PHALCON_INIT_VAR(automatic_attributes);
	PHALCON_CALL_METHOD_PARAMS_1(automatic_attributes, meta_data, "getautomaticupdateattributes", this_ptr);
	if (PHALCON_GLOBAL(orm).column_renaming) {
		PHALCON_INIT_VAR(column_map);
		PHALCON_CALL_METHOD_PARAMS_1(column_map, meta_data, "getcolumnmap", this_ptr);
	} else {
		PHALCON_INIT_NVAR(column_map);
	}
	
	/** 
	 * We only make the update based on the non-primary attributes, values in primary
	 * key attributes are ignored
	 */
	
	if (!phalcon_is_iterable(non_primary, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
		return;
	}
	
	while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
		PHALCON_GET_FOREACH_VALUE(field);
	
		if (!phalcon_array_isset(automatic_attributes, field)) {
	
			/** 
			 * Check a bind type for field to update
			 */
			if (!phalcon_array_isset(bind_data_types, field)) {
				PHALCON_INIT_NVAR(exception_message);
				PHALCON_CONCAT_SVS(exception_message, "Column '", field, "\" isn't part of the table columns");
				PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
				return;
			}
	
			/** 
			 * Check if the model has a column map
			 */
			if (Z_TYPE_P(column_map) == IS_ARRAY) { 
				if (phalcon_array_isset(column_map, field)) {
					PHALCON_OBS_NVAR(attribute_field);
					phalcon_array_fetch(&attribute_field, column_map, field, PH_NOISY_CC);
				} else {
					PHALCON_INIT_NVAR(exception_message);
					PHALCON_CONCAT_SVS(exception_message, "Column '", field, "\" isn't part of the column map");
					PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
					return;
				}
			} else {
				PHALCON_CPY_WRT(attribute_field, field);
			}
	
			/** 
			 * If a field isn't set we pass a null value
			 */
			if (phalcon_isset_property_zval(this_ptr, attribute_field TSRMLS_CC)) {
	
				/** 
				 * Get the field's value
				 */
				PHALCON_OBS_NVAR(value);
				phalcon_read_property_zval(&value, this_ptr, attribute_field, PH_NOISY_CC);
	
				/** 
				 * When dynamic update is not used we pass every field to the update
				 */
				if (!zend_is_true(use_dynamic_update)) {
					phalcon_array_append(&fields, field, PH_SEPARATE TSRMLS_CC);
					phalcon_array_append(&values, value, PH_SEPARATE TSRMLS_CC);
	
					PHALCON_OBS_NVAR(bind_type);
					phalcon_array_fetch(&bind_type, bind_data_types, field, PH_NOISY_CC);
					phalcon_array_append(&bind_types, bind_type, PH_SEPARATE TSRMLS_CC);
				} else {
					/** 
					 * If the field is not part of the snapshot we add them as changed
					 */
					if (!phalcon_array_isset(snapshot, attribute_field)) {
						PHALCON_INIT_NVAR(changed);
						ZVAL_BOOL(changed, 1);
					} else {
						PHALCON_OBS_NVAR(snapshot_value);
						phalcon_array_fetch(&snapshot_value, snapshot, attribute_field, PH_NOISY_CC);
						if (!PHALCON_IS_EQUAL(value, snapshot_value)) {
							PHALCON_INIT_NVAR(changed);
							ZVAL_BOOL(changed, 1);
						} else {
							PHALCON_INIT_NVAR(changed);
							ZVAL_BOOL(changed, 0);
						}
					}
	
					/** 
					 * Only changed values are added to the SQL Update
					 */
					if (zend_is_true(changed)) {
						phalcon_array_append(&fields, field, PH_SEPARATE TSRMLS_CC);
						phalcon_array_append(&values, value, PH_SEPARATE TSRMLS_CC);
	
						PHALCON_OBS_NVAR(bind_type);
						phalcon_array_fetch(&bind_type, bind_data_types, field, PH_NOISY_CC);
						phalcon_array_append(&bind_types, bind_type, PH_SEPARATE TSRMLS_CC);
					}
				}
			} else {
				phalcon_array_append(&fields, field, PH_SEPARATE TSRMLS_CC);
				phalcon_array_append(&values, null_value, PH_SEPARATE TSRMLS_CC);
				phalcon_array_append(&bind_types, bind_skip, PH_SEPARATE TSRMLS_CC);
			}
		}
	
		zend_hash_move_forward_ex(ah0, &hp0);
	}
	
	/** 
	 * If there is no fields to update we return true
	 */
	if (!phalcon_fast_count_ev(fields TSRMLS_CC)) {
		RETURN_MM_TRUE;
	}
	
	PHALCON_OBS_VAR(unique_key);
	phalcon_read_property(&unique_key, this_ptr, SL("_uniqueKey"), PH_NOISY_CC);
	
	PHALCON_OBS_VAR(unique_params);
	phalcon_read_property(&unique_params, this_ptr, SL("_uniqueParams"), PH_NOISY_CC);
	
	PHALCON_OBS_VAR(unique_types);
	phalcon_read_property(&unique_types, this_ptr, SL("_uniqueTypes"), PH_NOISY_CC);
	
	/** 
	 * When unique params is null we need to rebuild the bind params
	 */
	if (Z_TYPE_P(unique_params) != IS_ARRAY) { 
	
		PHALCON_INIT_NVAR(unique_params);
		array_init(unique_params);
	
		PHALCON_INIT_VAR(primary_keys);
		PHALCON_CALL_METHOD_PARAMS_1(primary_keys, meta_data, "getprimarykeyattributes", this_ptr);
	
		if (!phalcon_is_iterable(primary_keys, &ah1, &hp1, 0, 0 TSRMLS_CC)) {
			return;
		}
	
		while (zend_hash_get_current_data_ex(ah1, (void**) &hd, &hp1) == SUCCESS) {
	
			PHALCON_GET_FOREACH_VALUE(field);
	
			/** 
			 * Check if the model has a column map
			 */
			if (Z_TYPE_P(column_map) == IS_ARRAY) { 
				if (phalcon_array_isset(column_map, field)) {
					PHALCON_OBS_NVAR(attribute_field);
					phalcon_array_fetch(&attribute_field, column_map, field, PH_NOISY_CC);
				} else {
					PHALCON_INIT_NVAR(exception_message);
					PHALCON_CONCAT_SVS(exception_message, "Column '", field, "\" isn't part of the column map");
					PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
					return;
				}
			} else {
				PHALCON_CPY_WRT(attribute_field, field);
			}
			if (phalcon_isset_property_zval(this_ptr, attribute_field TSRMLS_CC)) {
				PHALCON_OBS_NVAR(value);
				phalcon_read_property_zval(&value, this_ptr, attribute_field, PH_NOISY_CC);
				phalcon_array_append(&unique_params, value, PH_SEPARATE TSRMLS_CC);
			} else {
				phalcon_array_append(&unique_params, null_value, PH_SEPARATE TSRMLS_CC);
			}
	
			zend_hash_move_forward_ex(ah1, &hp1);
		}
	
	}
	
	/** 
	 * We build the conditions as an array
	 */
	PHALCON_INIT_VAR(conditions);
	array_init_size(conditions, 3);
	phalcon_array_update_string(&conditions, SL("conditions"), &unique_key, PH_COPY | PH_SEPARATE TSRMLS_CC);
	phalcon_array_update_string(&conditions, SL("bind"), &unique_params, PH_COPY | PH_SEPARATE TSRMLS_CC);
	phalcon_array_update_string(&conditions, SL("bindTypes"), &unique_types, PH_COPY | PH_SEPARATE TSRMLS_CC);
	
	/** 
	 * Perform the low level update
	 */
	PHALCON_INIT_VAR(success);
	PHALCON_CALL_METHOD_PARAMS_5(success, connection, "update", table, fields, values, conditions, bind_types);
	
	RETURN_CCTOR(success);
}

/**
 *
 * @param array $related
 */
PHP_METHOD(Phalcon_Mvc_Model, _preSaveRelatedRecords){

	zval *connection, *related, *class_name, *manager;
	zval *record = NULL, *name = NULL, *relation = NULL, *type = NULL, *columns = NULL, *referenced_model = NULL;
	zval *referenced_fields = NULL, *status = NULL, *messages = NULL;
	zval *message = NULL, *referenced_value = NULL;
	HashTable *ah0, *ah1;
	HashPosition hp0, hp1;
	zval **hd;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &connection, &related) == FAILURE) {
		RETURN_MM_NULL();
	}

	/** 
	 * Start an implicit transaction
	 */
	PHALCON_CALL_METHOD_NORETURN(connection, "begin");
	
	PHALCON_INIT_VAR(class_name);
	phalcon_get_class(class_name, this_ptr, 0 TSRMLS_CC);
	
	PHALCON_INIT_VAR(manager);
	PHALCON_CALL_METHOD(manager, this_ptr, "getmodelsmanager");
	
	if (!phalcon_is_iterable(related, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
		return;
	}
	
	while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
		PHALCON_GET_FOREACH_KEY(name, ah0, hp0);
		PHALCON_GET_FOREACH_VALUE(record);
	
		/** 
		 * Try to get a relation with the same name
		 */
		PHALCON_INIT_NVAR(relation);
		PHALCON_CALL_METHOD_PARAMS_2(relation, manager, "getrelationbyalias", class_name, name);
		if (Z_TYPE_P(relation) == IS_OBJECT) {
	
			PHALCON_INIT_NVAR(type);
			PHALCON_CALL_METHOD(type, relation, "gettype");
	
			/** 
			 * Only belongsTo relations are important here
			 */
			if (PHALCON_IS_LONG(type, 0)) {
				if (Z_TYPE_P(record) != IS_OBJECT) {
					PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Only objects can be stored as part of belongs-to relations");
					return;
				}
	
				PHALCON_INIT_NVAR(columns);
				PHALCON_CALL_METHOD(columns, relation, "getfields");
	
				PHALCON_INIT_NVAR(referenced_model);
				PHALCON_CALL_METHOD(referenced_model, relation, "getreferencedmodel");
	
				PHALCON_INIT_NVAR(referenced_fields);
				PHALCON_CALL_METHOD(referenced_fields, relation, "getreferencedfields");
				if (Z_TYPE_P(columns) == IS_ARRAY) { 
					PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Not implemented");
					return;
				}
	
				PHALCON_INIT_NVAR(status);
				PHALCON_CALL_METHOD(status, record, "save");
				if (!zend_is_true(status)) {
	
					/** 
					 * Get the validation messages generated by the referenced model
					 */
					PHALCON_INIT_NVAR(messages);
					PHALCON_CALL_METHOD(messages, record, "getmessages");
	
					if (!phalcon_is_iterable(messages, &ah1, &hp1, 0, 0 TSRMLS_CC)) {
						return;
					}
	
					while (zend_hash_get_current_data_ex(ah1, (void**) &hd, &hp1) == SUCCESS) {
	
						PHALCON_GET_FOREACH_VALUE(message);
	
						/** 
						 * Set the related model
						 */
						PHALCON_CALL_METHOD_PARAMS_1_NORETURN(message, "setmodel", record);
	
						/** 
						 * Appends the messages to the current model
						 */
						PHALCON_CALL_METHOD_PARAMS_1_NORETURN(this_ptr, "appendmessage", message);
	
						zend_hash_move_forward_ex(ah1, &hp1);
					}
	
					/** 
					 * Rollback the implicit transaction
					 */
					PHALCON_CALL_METHOD_NORETURN(connection, "rollback");
					RETURN_MM_FALSE;
				}
	
				/** 
				 * Read the attribute from the referenced model and assigns it to the current model
				 */
				PHALCON_INIT_NVAR(referenced_value);
				PHALCON_CALL_METHOD_PARAMS_1(referenced_value, record, "readattribute", referenced_fields);
				phalcon_update_property_zval_zval(this_ptr, columns, referenced_value TSRMLS_CC);
			}
		}
	
		zend_hash_move_forward_ex(ah0, &hp0);
	}
	
	RETURN_MM_TRUE;
}

/**
 * Save the related records assigned in the has-one/has-many relations
 *
 * @param Phalcon\Db\AdapterInterface $connection
 * @param Phalcon\Mvc\ModelInterface[] $related
 */
PHP_METHOD(Phalcon_Mvc_Model, _postSaveRelatedRecords){

	zval *connection, *related, *class_name, *manager;
	zval *record = NULL, *name = NULL, *relation = NULL, *type = NULL, *columns = NULL, *referenced_model = NULL;
	zval *referenced_fields = NULL, *related_records = NULL;
	zval *exception_message = NULL, *value = NULL, *record_after = NULL;
	zval *status = NULL, *messages = NULL, *message = NULL;
	HashTable *ah0, *ah1, *ah2;
	HashPosition hp0, hp1, hp2;
	zval **hd;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &connection, &related) == FAILURE) {
		RETURN_MM_NULL();
	}

	PHALCON_INIT_VAR(class_name);
	phalcon_get_class(class_name, this_ptr, 0 TSRMLS_CC);
	
	PHALCON_INIT_VAR(manager);
	PHALCON_CALL_METHOD(manager, this_ptr, "getmodelsmanager");
	
	if (!phalcon_is_iterable(related, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
		return;
	}
	
	while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
		PHALCON_GET_FOREACH_KEY(name, ah0, hp0);
		PHALCON_GET_FOREACH_VALUE(record);
	
		/** 
		 * Try to get a relation with the same name
		 */
		PHALCON_INIT_NVAR(relation);
		PHALCON_CALL_METHOD_PARAMS_2(relation, manager, "getrelationbyalias", class_name, name);
		if (Z_TYPE_P(relation) == IS_OBJECT) {
	
			PHALCON_INIT_NVAR(type);
			PHALCON_CALL_METHOD(type, relation, "gettype");
	
			/** 
			 * Discard belongsTo relations
			 */
			if (PHALCON_IS_LONG(type, 0)) {
				zend_hash_move_forward_ex(ah0, &hp0);
				continue;
			}
	
			if (Z_TYPE_P(record) != IS_OBJECT) {
				if (Z_TYPE_P(record) != IS_ARRAY) { 
					PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Only objects/arrays can be stored as part of has-many/has-one relations");
					return;
				}
			}
	
			PHALCON_INIT_NVAR(columns);
			PHALCON_CALL_METHOD(columns, relation, "getfields");
	
			PHALCON_INIT_NVAR(referenced_model);
			PHALCON_CALL_METHOD(referenced_model, relation, "getreferencedmodel");
	
			PHALCON_INIT_NVAR(referenced_fields);
			PHALCON_CALL_METHOD(referenced_fields, relation, "getreferencedfields");
			if (Z_TYPE_P(columns) == IS_ARRAY) { 
				PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Not implemented");
				return;
			}
	
			/** 
			 * Create an implicit array of has-many/has-one records
			 */
			if (Z_TYPE_P(record) == IS_OBJECT) {
				PHALCON_INIT_NVAR(related_records);
				array_init_size(related_records, 1);
				phalcon_array_append(&related_records, record, PH_SEPARATE TSRMLS_CC);
			} else {
				PHALCON_CPY_WRT(related_records, record);
			}
	
			if (!phalcon_isset_property_zval(this_ptr, columns TSRMLS_CC)) {
				PHALCON_INIT_NVAR(exception_message);
				PHALCON_CONCAT_SVS(exception_message, "The column '", columns, "' needs to be present in the model");
				PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
				return;
			}
	
			/** 
			 * Get the value of the field from the current model
			 */
			PHALCON_OBS_NVAR(value);
			phalcon_read_property_zval(&value, this_ptr, columns, PH_NOISY_CC);
	
			if (!phalcon_is_iterable(related_records, &ah1, &hp1, 0, 0 TSRMLS_CC)) {
				return;
			}
	
			while (zend_hash_get_current_data_ex(ah1, (void**) &hd, &hp1) == SUCCESS) {
	
				PHALCON_GET_FOREACH_VALUE(record_after);
	
				/** 
				 * Assign the value to the 
				 */
				PHALCON_CALL_METHOD_PARAMS_2_NORETURN(record_after, "writeattribute", referenced_fields, value);
	
				PHALCON_INIT_NVAR(status);
				PHALCON_CALL_METHOD(status, record_after, "save");
				if (!zend_is_true(status)) {
	
					/** 
					 * Get the validation messages generated by the referenced model
					 */
					PHALCON_INIT_NVAR(messages);
					PHALCON_CALL_METHOD(messages, record_after, "getmessages");
	
					if (!phalcon_is_iterable(messages, &ah2, &hp2, 0, 0 TSRMLS_CC)) {
						return;
					}
	
					while (zend_hash_get_current_data_ex(ah2, (void**) &hd, &hp2) == SUCCESS) {
	
						PHALCON_GET_FOREACH_VALUE(message);
	
						/** 
						 * Set the related model
						 */
						PHALCON_CALL_METHOD_PARAMS_1_NORETURN(message, "setmodel", record);
	
						/** 
						 * Appends the messages to the current model
						 */
						PHALCON_CALL_METHOD_PARAMS_1_NORETURN(this_ptr, "appendmessage", message);
	
						zend_hash_move_forward_ex(ah2, &hp2);
					}
	
					/** 
					 * Rollback the implicit transaction
					 */
					PHALCON_CALL_METHOD_NORETURN(connection, "rollback");
					RETURN_MM_FALSE;
				}
	
				zend_hash_move_forward_ex(ah1, &hp1);
			}
	
		} else {
			if (Z_TYPE_P(record) != IS_ARRAY) { 
				PHALCON_INIT_NVAR(exception_message);
				PHALCON_CONCAT_SVSVS(exception_message, "There is no defined relations for the model \"", class_name, "\" using alias \"", name, "\"");
				PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
				return;
			}
		}
	
		zend_hash_move_forward_ex(ah0, &hp0);
	}
	
	/** 
	 * Commit the implicit transaction
	 */
	PHALCON_CALL_METHOD_NORETURN(connection, "commit");
	RETURN_MM_TRUE;
}

/**
 * Inserts or updates a model instance. Returning true on success or false otherwise.
 *
 *<code>
 *	//Creating a new robot
 *	$robot = new Robots();
 *	$robot->type = 'mechanical'
 *	$robot->name = 'Astro Boy';
 *	$robot->year = 1952;
 *	$robot->save();
 *
 *	//Updating a robot name
 *	$robot = Robots::findFirst("id=100");
 *	$robot->name = "Biomass";
 *	$robot->save();
 *</code>
 *
 * @param array $data
 * @param array $whiteList
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, save){

	zval *data = NULL, *white_list = NULL, *meta_data, *attributes;
	zval *attribute = NULL, *value = NULL, *possible_setter = NULL, *write_connection;
	zval *related, *status = NULL, *schema, *source, *table = NULL, *read_connection;
	zval *exists, *empty_array, *identity_field;
	zval *success = NULL;
	zval *r0 = NULL;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zz", &data, &white_list) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!data) {
		PHALCON_INIT_VAR(data);
	}
	
	if (!white_list) {
		PHALCON_INIT_VAR(white_list);
	}
	
	PHALCON_INIT_VAR(meta_data);
	PHALCON_CALL_METHOD(meta_data, this_ptr, "getmodelsmetadata");
	
	/** 
	 * Assign the values passed
	 */
	if (Z_TYPE_P(data) != IS_NULL) {
		if (Z_TYPE_P(data) != IS_ARRAY) { 
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Data passed to save() must be an array");
			return;
		}
	
		PHALCON_INIT_VAR(attributes);
		PHALCON_CALL_METHOD_PARAMS_1(attributes, meta_data, "getattributes", this_ptr);
	
		if (!phalcon_is_iterable(attributes, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
			return;
		}
	
		while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
			PHALCON_GET_FOREACH_VALUE(attribute);
	
			if (phalcon_array_isset(data, attribute)) {
	
				/** 
				 * If the white-list is an array check if the attribute is on that list
				 */
				if (Z_TYPE_P(white_list) == IS_ARRAY) { 
					if (!phalcon_array_isset(white_list, attribute)) {
						zend_hash_move_forward_ex(ah0, &hp0);
						continue;
					}
				}
	
				/** 
				 * We check if the field has a setter
				 */
				PHALCON_OBS_NVAR(value);
				phalcon_array_fetch(&value, data, attribute, PH_NOISY_CC);
	
				PHALCON_INIT_NVAR(possible_setter);
				PHALCON_CONCAT_SV(possible_setter, "set", attribute);
				if (phalcon_method_exists(this_ptr, possible_setter TSRMLS_CC) == SUCCESS) {
					PHALCON_CALL_METHOD_PARAMS_1_NORETURN(this_ptr, Z_STRVAL_P(possible_setter), value);
				} else {
					/** 
					 * Otherwise we assign the attribute directly
					 */
					phalcon_update_property_zval_zval(this_ptr, attribute, value TSRMLS_CC);
				}
			}
	
			zend_hash_move_forward_ex(ah0, &hp0);
		}
	
	}
	
	/** 
	 * Create/Get the current database connection
	 */
	PHALCON_INIT_VAR(write_connection);
	PHALCON_CALL_METHOD(write_connection, this_ptr, "getwriteconnection");
	
	/** 
	 * Save related records in belongsTo relationships
	 */
	PHALCON_OBS_VAR(related);
	phalcon_read_property(&related, this_ptr, SL("_related"), PH_NOISY_CC);
	if (Z_TYPE_P(related) == IS_ARRAY) { 
	
		PHALCON_INIT_VAR(status);
		PHALCON_CALL_METHOD_PARAMS_2(status, this_ptr, "_presaverelatedrecords", write_connection, related);
		if (PHALCON_IS_FALSE(status)) {
			RETURN_MM_FALSE;
		}
	}
	
	PHALCON_INIT_VAR(schema);
	PHALCON_CALL_METHOD(schema, this_ptr, "getschema");
	
	PHALCON_INIT_VAR(source);
	PHALCON_CALL_METHOD(source, this_ptr, "getsource");
	if (zend_is_true(schema)) {
		PHALCON_INIT_VAR(table);
		array_init_size(table, 2);
		phalcon_array_append(&table, schema, PH_SEPARATE TSRMLS_CC);
		phalcon_array_append(&table, source, PH_SEPARATE TSRMLS_CC);
	} else {
		PHALCON_CPY_WRT(table, source);
	}
	
	/** 
	 * Create/Get the current database connection
	 */
	PHALCON_INIT_VAR(read_connection);
	PHALCON_CALL_METHOD(read_connection, this_ptr, "getreadconnection");
	
	/** 
	 * We need to check if the record exists
	 */
	PHALCON_INIT_VAR(exists);
	PHALCON_CALL_METHOD_PARAMS_3(exists, this_ptr, "_exists", meta_data, read_connection, table);
	if (zend_is_true(exists)) {
		phalcon_update_property_long(this_ptr, SL("_operationMade"), 2 TSRMLS_CC);
	} else {
		phalcon_update_property_long(this_ptr, SL("_operationMade"), 1 TSRMLS_CC);
	}
	
	/** 
	 * Clean the messages
	 */
	PHALCON_INIT_VAR(empty_array);
	array_init(empty_array);
	phalcon_update_property_zval(this_ptr, SL("_errorMessages"), empty_array TSRMLS_CC);
	
	/** 
	 * Query the identity field
	 */
	PHALCON_INIT_VAR(identity_field);
	PHALCON_CALL_METHOD_PARAMS_1(identity_field, meta_data, "getidentityfield", this_ptr);
	
	/** 
	 * _preSave() makes all the validations
	 */
	PHALCON_INIT_NVAR(status);
	PHALCON_CALL_METHOD_PARAMS_3(status, this_ptr, "_presave", meta_data, exists, identity_field);
	if (PHALCON_IS_FALSE(status)) {
	
		/** 
		 * Rollback the current transaction if there was validation errors
		 */
		if (Z_TYPE_P(related) == IS_ARRAY) { 
			PHALCON_CALL_METHOD_NORETURN(write_connection, "rollback");
		}
		RETURN_MM_FALSE;
	}
	
	/** 
	 * Depending if the record exists we do an update or an insert operation
	 */
	if (zend_is_true(exists)) {
		PHALCON_INIT_VAR(success);
		PHALCON_CALL_METHOD_PARAMS_3(success, this_ptr, "_dolowupdate", meta_data, write_connection, table);
	} else {
		PHALCON_INIT_NVAR(success);
		PHALCON_CALL_METHOD_PARAMS_4(success, this_ptr, "_dolowinsert", meta_data, write_connection, table, identity_field);
	}
	
	/** 
	 * Change the dirty state to persistent
	 */
	if (zend_is_true(success)) {
		phalcon_update_property_long(this_ptr, SL("_dirtyState"), 0 TSRMLS_CC);
	}
	
	/** 
	 * _postSave() makes all the validations
	 */
	if (PHALCON_GLOBAL(orm).events) {
		PHALCON_INIT_VAR(r0);
		PHALCON_CALL_METHOD_PARAMS_2(r0, this_ptr, "_postsave", success, exists);
		PHALCON_CPY_WRT(success, r0);
	}
	
	if (Z_TYPE_P(related) == IS_ARRAY) { 
	
		/** 
		 * Rollbacks the implicit transaction if the master save has failed
		 */
		if (PHALCON_IS_FALSE(success)) {
			PHALCON_CALL_METHOD_NORETURN(write_connection, "rollback");
			RETURN_MM_FALSE;
		}
	
		/** 
		 * Save the post-related records
		 */
		PHALCON_INIT_NVAR(status);
		PHALCON_CALL_METHOD_PARAMS_2(status, this_ptr, "_postsaverelatedrecords", write_connection, related);
		if (PHALCON_IS_FALSE(status)) {
			RETURN_MM_FALSE;
		}
	}
	
	
	RETURN_CCTOR(success);
}

/**
 * Inserts a model instance. If the instance already exists in the persistance it will throw an exception
 * Returning true on success or false otherwise.
 *
 *<code>
 *	//Creating a new robot
 *	$robot = new Robots();
 *	$robot->type = 'mechanical'
 *	$robot->name = 'Astro Boy';
 *	$robot->year = 1952;
 *	$robot->create();
 *
 *  //Passing an array to create
 *  $robot = new Robots();
 *  $robot->create(array(
 *      'type' => 'mechanical',
 *      'name' => 'Astroy Boy',
 *      'year' => 1952
 *  ));
 *</code>
 *
 * @param array $data
 * @param array $whiteList
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, create){

	zval *data = NULL, *white_list = NULL, *meta_data, *column_map = NULL;
	zval *attributes, *attribute = NULL, *attribute_field = NULL;
	zval *exception_message = NULL, *value = NULL, *possible_setter = NULL;
	zval *read_connection, *exists, *field, *type, *message;
	zval *model_message, *messages, *success;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zz", &data, &white_list) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!data) {
		PHALCON_INIT_VAR(data);
	}
	
	if (!white_list) {
		PHALCON_INIT_VAR(white_list);
	}
	
	PHALCON_INIT_VAR(meta_data);
	PHALCON_CALL_METHOD(meta_data, this_ptr, "getmodelsmetadata");
	
	/** 
	 * Assign the values passed
	 */
	if (Z_TYPE_P(data) != IS_NULL) {
		if (Z_TYPE_P(data) != IS_ARRAY) { 
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Data passed to create() must be an array");
			return;
		}
		if (PHALCON_GLOBAL(orm).column_renaming) {
			PHALCON_INIT_VAR(column_map);
			PHALCON_CALL_METHOD_PARAMS_1(column_map, meta_data, "getcolumnmap", this_ptr);
		} else {
			PHALCON_INIT_NVAR(column_map);
		}
	
		/** 
		 * We assign the fields starting from the current attributes in the model
		 */
		PHALCON_INIT_VAR(attributes);
		PHALCON_CALL_METHOD_PARAMS_1(attributes, meta_data, "getattributes", this_ptr);
	
		if (!phalcon_is_iterable(attributes, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
			return;
		}
	
		while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
			PHALCON_GET_FOREACH_VALUE(attribute);
	
			/** 
			 * Check if we need to rename the field
			 */
			if (Z_TYPE_P(column_map) == IS_ARRAY) { 
				if (phalcon_array_isset(column_map, attribute)) {
					PHALCON_OBS_NVAR(attribute_field);
					phalcon_array_fetch(&attribute_field, column_map, attribute, PH_NOISY_CC);
				} else {
					PHALCON_INIT_NVAR(exception_message);
					PHALCON_CONCAT_SVS(exception_message, "Column '", attribute, "\" isn't part of the column map");
					PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
					return;
				}
			} else {
				PHALCON_CPY_WRT(attribute_field, attribute);
			}
	
			/** 
			 * Check if we there is data for the field
			 */
			if (phalcon_array_isset(data, attribute_field)) {
	
				/** 
				 * If the white-list is an array check if the attribute is on that list
				 */
				if (Z_TYPE_P(white_list) == IS_ARRAY) { 
					if (!phalcon_array_isset(white_list, attribute_field)) {
						zend_hash_move_forward_ex(ah0, &hp0);
						continue;
					}
				}
	
				/** 
				 * The value in the array passed
				 */
				PHALCON_OBS_NVAR(value);
				phalcon_array_fetch(&value, data, attribute_field, PH_NOISY_CC);
	
				/** 
				 * Check if the field has a posible setter
				 */
				PHALCON_INIT_NVAR(possible_setter);
				PHALCON_CONCAT_SV(possible_setter, "set", attribute_field);
				if (phalcon_method_exists(this_ptr, possible_setter TSRMLS_CC) == SUCCESS) {
					PHALCON_CALL_METHOD_PARAMS_1_NORETURN(this_ptr, Z_STRVAL_P(possible_setter), value);
				} else {
					phalcon_update_property_zval_zval(this_ptr, attribute_field, value TSRMLS_CC);
				}
			}
	
			zend_hash_move_forward_ex(ah0, &hp0);
		}
	
	}
	
	/** 
	 * Get the current connection
	 */
	PHALCON_INIT_VAR(read_connection);
	PHALCON_CALL_METHOD(read_connection, this_ptr, "getreadconnection");
	
	/** 
	 * A 'exists' confirmation is performed first
	 */
	PHALCON_INIT_VAR(exists);
	PHALCON_CALL_METHOD_PARAMS_2(exists, this_ptr, "_exists", meta_data, read_connection);
	
	/** 
	 * If the record already exists we must throw an exception
	 */
	if (zend_is_true(exists)) {
		PHALCON_INIT_VAR(field);
	
		PHALCON_INIT_VAR(type);
		ZVAL_STRING(type, "InvalidCreateAttempt", 1);
	
		PHALCON_INIT_VAR(message);
		ZVAL_STRING(message, "Record cannot be created because it already exists", 1);
	
		PHALCON_INIT_VAR(model_message);
		object_init_ex(model_message, phalcon_mvc_model_message_ce);
		PHALCON_CALL_METHOD_PARAMS_3_NORETURN(model_message, "__construct", message, field, type);
	
		PHALCON_INIT_VAR(messages);
		array_init_size(messages, 1);
		phalcon_array_append(&messages, model_message, PH_SEPARATE TSRMLS_CC);
		phalcon_update_property_zval(this_ptr, SL("_errorMessages"), messages TSRMLS_CC);
		RETURN_MM_FALSE;
	}
	
	/** 
	 * Using save() anyways
	 */
	PHALCON_INIT_VAR(success);
	PHALCON_CALL_METHOD(success, this_ptr, "save");
	
	RETURN_CCTOR(success);
}

/**
 * Updates a model instance. If the instance doesn't exist in the persistance it will throw an exception
 * Returning true on success or false otherwise.
 *
 *<code>
 *	//Updating a robot name
 *	$robot = Robots::findFirst("id=100");
 *	$robot->name = "Biomass";
 *	$robot->update();
 *</code>
 *
 * @param array $data
 * @param array $whiteList
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, update){

	zval *data = NULL, *white_list = NULL, *meta_data = NULL, *column_map = NULL;
	zval *attributes, *attribute = NULL, *attribute_field = NULL;
	zval *exception_message = NULL, *value = NULL, *possible_setter = NULL;
	zval *dirty_state, *read_connection, *exists;
	zval *field, *type, *message, *model_message, *messages;
	zval *success;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zz", &data, &white_list) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!data) {
		PHALCON_INIT_VAR(data);
	}
	
	if (!white_list) {
		PHALCON_INIT_VAR(white_list);
	}
	
	PHALCON_INIT_VAR(meta_data);
	
	/** 
	 * Assign the values bassed on the passed
	 */
	if (Z_TYPE_P(data) != IS_NULL) {
		if (Z_TYPE_P(data) != IS_ARRAY) { 
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Data passed to update() must be an array");
			return;
		}
	
		PHALCON_CALL_METHOD(meta_data, this_ptr, "getmodelsmetadata");
		if (PHALCON_GLOBAL(orm).column_renaming) {
			PHALCON_INIT_VAR(column_map);
			PHALCON_CALL_METHOD_PARAMS_1(column_map, meta_data, "getcolumnmap", this_ptr);
		} else {
			PHALCON_INIT_NVAR(column_map);
		}
	
		/** 
		 * We assign the fields starting from the current attributes in the model
		 */
		PHALCON_INIT_VAR(attributes);
		PHALCON_CALL_METHOD_PARAMS_1(attributes, meta_data, "getattributes", this_ptr);
	
		if (!phalcon_is_iterable(attributes, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
			return;
		}
	
		while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
			PHALCON_GET_FOREACH_VALUE(attribute);
	
			/** 
			 * Check if we need to rename the field
			 */
			if (Z_TYPE_P(column_map) == IS_ARRAY) { 
				if (phalcon_array_isset(column_map, attribute)) {
					PHALCON_OBS_NVAR(attribute_field);
					phalcon_array_fetch(&attribute_field, column_map, attribute, PH_NOISY_CC);
				} else {
					PHALCON_INIT_NVAR(exception_message);
					PHALCON_CONCAT_SVS(exception_message, "Column '", attribute, "\" isn't part of the column map");
					PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
					return;
				}
			} else {
				PHALCON_CPY_WRT(attribute_field, attribute);
			}
	
			/** 
			 * Check if we there is data for the field
			 */
			if (phalcon_array_isset(data, attribute_field)) {
	
				/** 
				 * If the white-list is an array check if the attribute is on that list
				 */
				if (Z_TYPE_P(white_list) == IS_ARRAY) { 
					if (!phalcon_array_isset(white_list, attribute_field)) {
						zend_hash_move_forward_ex(ah0, &hp0);
						continue;
					}
				}
	
				/** 
				 * Reads the attribute from the data
				 */
				PHALCON_OBS_NVAR(value);
				phalcon_array_fetch(&value, data, attribute_field, PH_NOISY_CC);
	
				/** 
				 * Try to find a possible getter
				 */
				PHALCON_INIT_NVAR(possible_setter);
				PHALCON_CONCAT_SV(possible_setter, "set", attribute_field);
				if (phalcon_method_exists(this_ptr, possible_setter TSRMLS_CC) == SUCCESS) {
					PHALCON_CALL_METHOD_PARAMS_1_NORETURN(this_ptr, Z_STRVAL_P(possible_setter), value);
				} else {
					phalcon_update_property_zval_zval(this_ptr, attribute_field, value TSRMLS_CC);
				}
			}
	
			zend_hash_move_forward_ex(ah0, &hp0);
		}
	
	}
	
	/** 
	 * We don't check if the record exists if the record is already checked
	 */
	PHALCON_OBS_VAR(dirty_state);
	phalcon_read_property(&dirty_state, this_ptr, SL("_dirtyState"), PH_NOISY_CC);
	if (zend_is_true(dirty_state)) {
		if (Z_TYPE_P(meta_data) == IS_NULL) {
			PHALCON_INIT_NVAR(meta_data);
			PHALCON_CALL_METHOD(meta_data, this_ptr, "getmodelsmetadata");
		}
	
		PHALCON_INIT_VAR(read_connection);
		PHALCON_CALL_METHOD(read_connection, this_ptr, "getreadconnection");
	
		PHALCON_INIT_VAR(exists);
		PHALCON_CALL_METHOD_PARAMS_2(exists, this_ptr, "_exists", meta_data, read_connection);
		if (!zend_is_true(exists)) {
			PHALCON_INIT_VAR(field);
	
			PHALCON_INIT_VAR(type);
			ZVAL_STRING(type, "InvalidUpdateAttempt", 1);
	
			PHALCON_INIT_VAR(message);
			ZVAL_STRING(message, "Record cannot be updated because it does not exist", 1);
	
			PHALCON_INIT_VAR(model_message);
			object_init_ex(model_message, phalcon_mvc_model_message_ce);
			PHALCON_CALL_METHOD_PARAMS_3_NORETURN(model_message, "__construct", message, field, type);
	
			PHALCON_INIT_VAR(messages);
			array_init_size(messages, 1);
			phalcon_array_append(&messages, model_message, PH_SEPARATE TSRMLS_CC);
			phalcon_update_property_zval(this_ptr, SL("_errorMessages"), messages TSRMLS_CC);
			RETURN_MM_FALSE;
		}
	}
	
	/** 
	 * Call save() anyways
	 */
	PHALCON_INIT_VAR(success);
	PHALCON_CALL_METHOD(success, this_ptr, "save");
	
	RETURN_CCTOR(success);
}

/**
 * Deletes a model instance. Returning true on success or false otherwise.
 *
 * <code>
 *$robot = Robots::findFirst("id=100");
 *$robot->delete();
 *
 *foreach(Robots::find("type = 'mechanical'") as $robot){
 *   $robot->delete();
 *}
 * </code>
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, delete){

	zval *meta_data, *write_connection, *empty_array;
	zval *check_foreign_keys, *values, *bind_types;
	zval *conditions, *primary_keys, *bind_data_types;
	zval *column_map = NULL, *primary_key = NULL, *exception_message = NULL;
	zval *attribute_field = NULL, *value = NULL, *escaped_field = NULL;
	zval *primary_condition = NULL, *bind_type = NULL, *delete_conditions;
	zval *event_name = NULL, *status, *skipped, *schema, *source;
	zval *table = NULL, *success;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	PHALCON_INIT_VAR(meta_data);
	PHALCON_CALL_METHOD(meta_data, this_ptr, "getmodelsmetadata");
	
	PHALCON_INIT_VAR(write_connection);
	PHALCON_CALL_METHOD(write_connection, this_ptr, "getwriteconnection");
	
	/** 
	 * Operation made is OP_DELETE
	 */
	phalcon_update_property_long(this_ptr, SL("_operationMade"), 3 TSRMLS_CC);
	
	PHALCON_INIT_VAR(empty_array);
	array_init(empty_array);
	phalcon_update_property_zval(this_ptr, SL("_errorMessages"), empty_array TSRMLS_CC);
	
	/** 
	 * Check if deleting the record violates a virtual foreign key
	 */
	if (PHALCON_GLOBAL(orm).virtual_foreign_keys) {
	
		PHALCON_INIT_VAR(check_foreign_keys);
		PHALCON_CALL_METHOD(check_foreign_keys, this_ptr, "_checkforeignkeysreverse");
		if (PHALCON_IS_FALSE(check_foreign_keys)) {
			RETURN_MM_FALSE;
		}
	}
	
	PHALCON_INIT_VAR(values);
	array_init(values);
	
	PHALCON_INIT_VAR(bind_types);
	array_init(bind_types);
	
	PHALCON_INIT_VAR(conditions);
	array_init(conditions);
	
	PHALCON_INIT_VAR(primary_keys);
	PHALCON_CALL_METHOD_PARAMS_1(primary_keys, meta_data, "getprimarykeyattributes", this_ptr);
	
	PHALCON_INIT_VAR(bind_data_types);
	PHALCON_CALL_METHOD_PARAMS_1(bind_data_types, meta_data, "getbindtypes", this_ptr);
	if (PHALCON_GLOBAL(orm).column_renaming) {
		PHALCON_INIT_VAR(column_map);
		PHALCON_CALL_METHOD_PARAMS_1(column_map, meta_data, "getcolumnmap", this_ptr);
	} else {
		PHALCON_INIT_NVAR(column_map);
	}
	
	/** 
	 * Create a condition from the primary keys
	 */
	
	if (!phalcon_is_iterable(primary_keys, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
		return;
	}
	
	while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
		PHALCON_GET_FOREACH_VALUE(primary_key);
	
		/** 
		 * Every column part of the primary key must be in the bind data types
		 */
		if (!phalcon_array_isset(bind_data_types, primary_key)) {
			PHALCON_INIT_NVAR(exception_message);
			PHALCON_CONCAT_SVS(exception_message, "Column '", primary_key, "\" isn't part of the table columns");
			PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
			return;
		}
	
		/** 
		 * Take the column values based on the column map if any
		 */
		if (Z_TYPE_P(column_map) == IS_ARRAY) { 
			if (phalcon_array_isset(column_map, primary_key)) {
				PHALCON_OBS_NVAR(attribute_field);
				phalcon_array_fetch(&attribute_field, column_map, primary_key, PH_NOISY_CC);
			} else {
				PHALCON_INIT_NVAR(exception_message);
				PHALCON_CONCAT_SVS(exception_message, "Column '", primary_key, "\" isn't part of the column map");
				PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
				return;
			}
		} else {
			PHALCON_CPY_WRT(attribute_field, primary_key);
		}
	
		/** 
		 * If the attribute is currently set in the object add it to the conditions
		 */
		if (!phalcon_isset_property_zval(this_ptr, attribute_field TSRMLS_CC)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Cannot delete the record because one of the primary key attributes isn't set");
			return;
		}
	
		PHALCON_OBS_NVAR(value);
		phalcon_read_property_zval(&value, this_ptr, attribute_field, PH_NOISY_CC);
		phalcon_array_append(&values, value, PH_SEPARATE TSRMLS_CC);
	
		/** 
		 * Escape the column identifier
		 */
		PHALCON_INIT_NVAR(escaped_field);
		PHALCON_CALL_METHOD_PARAMS_1(escaped_field, write_connection, "escapeidentifier", primary_key);
	
		PHALCON_INIT_NVAR(primary_condition);
		PHALCON_CONCAT_VS(primary_condition, escaped_field, " = ?");
		phalcon_array_append(&conditions, primary_condition, PH_SEPARATE TSRMLS_CC);
	
		PHALCON_OBS_NVAR(bind_type);
		phalcon_array_fetch(&bind_type, bind_data_types, primary_key, PH_NOISY_CC);
		phalcon_array_append(&bind_types, bind_type, PH_SEPARATE TSRMLS_CC);
	
		zend_hash_move_forward_ex(ah0, &hp0);
	}
	
	/** 
	 * Join the conditions in the array using an AND operator
	 */
	PHALCON_INIT_VAR(delete_conditions);
	phalcon_fast_join_str(delete_conditions, SL(" AND "), conditions TSRMLS_CC);
	if (PHALCON_GLOBAL(orm).events) {
		phalcon_update_property_bool(this_ptr, SL("_skipped"), 0 TSRMLS_CC);
	
		PHALCON_INIT_VAR(event_name);
		ZVAL_STRING(event_name, "beforeDelete", 1);
	
		PHALCON_INIT_VAR(status);
		PHALCON_CALL_METHOD_PARAMS_1(status, this_ptr, "fireeventcancel", event_name);
		if (PHALCON_IS_FALSE(status)) {
			RETURN_MM_FALSE;
		} else {
			/** 
			 * The operation can be skipped
			 */
			PHALCON_OBS_VAR(skipped);
			phalcon_read_property(&skipped, this_ptr, SL("_skipped"), PH_NOISY_CC);
			if (PHALCON_IS_TRUE(skipped)) {
				RETURN_MM_TRUE;
			}
		}
	}
	
	PHALCON_INIT_VAR(schema);
	PHALCON_CALL_METHOD(schema, this_ptr, "getschema");
	
	PHALCON_INIT_VAR(source);
	PHALCON_CALL_METHOD(source, this_ptr, "getsource");
	if (zend_is_true(schema)) {
		PHALCON_INIT_VAR(table);
		array_init_size(table, 2);
		phalcon_array_append(&table, schema, PH_SEPARATE TSRMLS_CC);
		phalcon_array_append(&table, source, PH_SEPARATE TSRMLS_CC);
	} else {
		PHALCON_CPY_WRT(table, source);
	}
	
	/** 
	 * Do the deletion
	 */
	PHALCON_INIT_VAR(success);
	PHALCON_CALL_METHOD_PARAMS_4(success, write_connection, "delete", table, delete_conditions, values, bind_types);
	if (PHALCON_GLOBAL(orm).events) {
		if (zend_is_true(success)) {
			PHALCON_INIT_NVAR(event_name);
			ZVAL_STRING(event_name, "afterDelete", 1);
			PHALCON_CALL_METHOD_PARAMS_1_NORETURN(this_ptr, "fireevent", event_name);
		}
	}
	
	/** 
	 * Force perform the record existence checking again
	 */
	phalcon_update_property_long(this_ptr, SL("_dirtyState"), 2 TSRMLS_CC);
	
	RETURN_CCTOR(success);
}

/**
 * Returns the type of the latest operation performed by the ORM
 * Returns one of the OP_* class constants
 *
 * @return int
 */
PHP_METHOD(Phalcon_Mvc_Model, getOperationMade){


	RETURN_MEMBER(this_ptr, "_operationMade");
}

/**
 * Refreshes the model attributes re-querying the record from the database
 */
PHP_METHOD(Phalcon_Mvc_Model, refresh){

	zval *dirty_state, *meta_data, *read_connection;
	zval *schema, *source, *table = NULL, *unique_key = NULL, *exists;
	zval *unique_params, *unique_types, *attributes;
	zval *fields, *attribute = NULL, *field_item = NULL, *escaped_table;
	zval *select, *dialect, *sql, *fetch_type, *row, *column_map;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	PHALCON_OBS_VAR(dirty_state);
	phalcon_read_property(&dirty_state, this_ptr, SL("_dirtyState"), PH_NOISY_CC);
	if (!PHALCON_IS_LONG(dirty_state, 0)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The record cannot be refreshed because it does not exist or is deleted");
		return;
	}
	
	PHALCON_INIT_VAR(meta_data);
	PHALCON_CALL_METHOD(meta_data, this_ptr, "getmodelsmetadata");
	
	PHALCON_INIT_VAR(read_connection);
	PHALCON_CALL_METHOD(read_connection, this_ptr, "getreadconnection");
	
	PHALCON_INIT_VAR(schema);
	PHALCON_CALL_METHOD(schema, this_ptr, "getschema");
	
	PHALCON_INIT_VAR(source);
	PHALCON_CALL_METHOD(source, this_ptr, "getsource");
	if (zend_is_true(schema)) {
		PHALCON_INIT_VAR(table);
		array_init_size(table, 2);
		phalcon_array_append(&table, schema, PH_SEPARATE TSRMLS_CC);
		phalcon_array_append(&table, source, PH_SEPARATE TSRMLS_CC);
	} else {
		PHALCON_CPY_WRT(table, source);
	}
	
	PHALCON_OBS_VAR(unique_key);
	phalcon_read_property(&unique_key, this_ptr, SL("_uniqueKey"), PH_NOISY_CC);
	if (!zend_is_true(unique_key)) {
	
		/** 
		 * We need to check if the record exists
		 */
		PHALCON_INIT_VAR(exists);
		PHALCON_CALL_METHOD_PARAMS_3(exists, this_ptr, "_exists", meta_data, read_connection, table);
		if (!zend_is_true(exists)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The record cannot be refreshed because it does not exist or is deleted");
			return;
		}
	
		PHALCON_OBS_NVAR(unique_key);
		phalcon_read_property(&unique_key, this_ptr, SL("_uniqueKey"), PH_NOISY_CC);
	}
	
	PHALCON_OBS_VAR(unique_params);
	phalcon_read_property(&unique_params, this_ptr, SL("_uniqueParams"), PH_NOISY_CC);
	if (Z_TYPE_P(unique_params) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The record cannot be refreshed because it does not exist or is deleted");
		return;
	}
	
	PHALCON_OBS_VAR(unique_types);
	phalcon_read_property(&unique_types, this_ptr, SL("_uniqueTypes"), PH_NOISY_CC);
	
	/** 
	 * We only refresh the attributes in the model's metadata
	 */
	PHALCON_INIT_VAR(attributes);
	PHALCON_CALL_METHOD_PARAMS_1(attributes, meta_data, "getattributes", this_ptr);
	
	PHALCON_INIT_VAR(fields);
	array_init(fields);
	
	if (!phalcon_is_iterable(attributes, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
		return;
	}
	
	while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
		PHALCON_GET_FOREACH_VALUE(attribute);
	
		PHALCON_INIT_NVAR(field_item);
		array_init_size(field_item, 1);
		phalcon_array_append(&field_item, attribute, PH_SEPARATE TSRMLS_CC);
		phalcon_array_append(&fields, field_item, PH_SEPARATE TSRMLS_CC);
	
		zend_hash_move_forward_ex(ah0, &hp0);
	}
	
	PHALCON_INIT_VAR(escaped_table);
	PHALCON_CALL_METHOD_PARAMS_1(escaped_table, read_connection, "escapeidentifier", table);
	
	PHALCON_INIT_VAR(select);
	array_init(select);
	phalcon_array_update_string(&select, SL("columns"), &fields, PH_COPY | PH_SEPARATE TSRMLS_CC);
	phalcon_array_update_string(&select, SL("tables"), &escaped_table, PH_COPY | PH_SEPARATE TSRMLS_CC);
	phalcon_array_update_string(&select, SL("where"), &unique_key, PH_COPY | PH_SEPARATE TSRMLS_CC);
	
	/** 
	 * We directly build the SELECT to save resources
	 */
	PHALCON_INIT_VAR(dialect);
	PHALCON_CALL_METHOD(dialect, read_connection, "getdialect");
	
	PHALCON_INIT_VAR(sql);
	PHALCON_CALL_METHOD_PARAMS_1(sql, dialect, "select", select);
	
	PHALCON_INIT_VAR(fetch_type);
	ZVAL_LONG(fetch_type, 1);
	
	PHALCON_INIT_VAR(row);
	PHALCON_CALL_METHOD_PARAMS_4(row, read_connection, "fetchone", sql, fetch_type, unique_params, unique_types);
	
	/** 
	 * Get a column map if any
	 */
	PHALCON_INIT_VAR(column_map);
	PHALCON_CALL_METHOD_PARAMS_1(column_map, meta_data, "getcolumnmap", this_ptr);
	
	/** 
	 * Assign the resulting array to the this_ptr object
	 */
	if (Z_TYPE_P(row) == IS_ARRAY) { 
		PHALCON_CALL_METHOD_PARAMS_2_NORETURN(this_ptr, "assign", row, column_map);
	}
	
	PHALCON_MM_RESTORE();
}

/**
 * Skips the current operation forcing a success state
 *
 * @param boolean $skip
 */
PHP_METHOD(Phalcon_Mvc_Model, skipOperation){

	zval *skip;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &skip) == FAILURE) {
		RETURN_NULL();
	}

	phalcon_update_property_zval(this_ptr, SL("_skipped"), skip TSRMLS_CC);
	
}

/**
 * Reads an attribute value by its name
 *
 * <code>
 * echo $robot->readAttribute('name');
 * </code>
 *
 * @param string $attribute
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Model, readAttribute){

	zval *attribute, *attribute_value;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &attribute) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (phalcon_isset_property_zval(this_ptr, attribute TSRMLS_CC)) {
		PHALCON_OBS_VAR(attribute_value);
		phalcon_read_property_zval(&attribute_value, this_ptr, attribute, PH_NOISY_CC);
		RETURN_CCTOR(attribute_value);
	}
	RETURN_MM_NULL();
}

/**
 * Writes an attribute value by its name
 *
 * <code>
 * 	$robot->writeAttribute('name', 'Rosey');
 * </code>
 *
 * @param string $attribute
 * @param mixed $value
 */
PHP_METHOD(Phalcon_Mvc_Model, writeAttribute){

	zval *attribute, *value;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &attribute, &value) == FAILURE) {
		RETURN_NULL();
	}

	phalcon_update_property_zval_zval(this_ptr, attribute, value TSRMLS_CC);
	
}

/**
 * Sets a list of attributes that must be skipped from the
 * generated INSERT/UPDATE statement
 *
 *<code>
 *
 *class Robots extends \Phalcon\Mvc\Model
 *{
 *
 *   public function initialize()
 *   {
 *       $this->skipAttributes(array('price'));
 *   }
 *
 *}
 *</code>
 *
 * @param array $attributes
 */
PHP_METHOD(Phalcon_Mvc_Model, skipAttributes){

	zval *attributes, *null_value, *keys_attributes;
	zval *attribute = NULL, *meta_data;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &attributes) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (Z_TYPE_P(attributes) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Attributes must be an array");
		return;
	}
	
	PHALCON_INIT_VAR(null_value);
	
	PHALCON_INIT_VAR(keys_attributes);
	array_init(keys_attributes);
	
	if (!phalcon_is_iterable(attributes, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
		return;
	}
	
	while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
		PHALCON_GET_FOREACH_VALUE(attribute);
	
		phalcon_array_update_zval(&keys_attributes, attribute, &null_value, PH_COPY | PH_SEPARATE TSRMLS_CC);
	
		zend_hash_move_forward_ex(ah0, &hp0);
	}
	
	PHALCON_INIT_VAR(meta_data);
	PHALCON_CALL_METHOD(meta_data, this_ptr, "getmodelsmetadata");
	PHALCON_CALL_METHOD_PARAMS_2_NORETURN(meta_data, "setautomaticcreateattributes", this_ptr, keys_attributes);
	PHALCON_CALL_METHOD_PARAMS_2_NORETURN(meta_data, "setautomaticupdateattributes", this_ptr, keys_attributes);
	
	PHALCON_MM_RESTORE();
}

/**
 * Sets a list of attributes that must be skipped from the
 * generated INSERT statement
 *
 *<code>
 *
 *class Robots extends \Phalcon\Mvc\Model
 *{
 *
 *   public function initialize()
 *   {
 *       $this->skipAttributesOnCreate(array('created_at'));
 *   }
 *
 *}
 *</code>
 *
 * @param array $attributes
 */
PHP_METHOD(Phalcon_Mvc_Model, skipAttributesOnCreate){

	zval *attributes, *null_value, *keys_attributes;
	zval *attribute = NULL, *meta_data;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &attributes) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (Z_TYPE_P(attributes) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Attributes must be an array");
		return;
	}
	
	PHALCON_INIT_VAR(null_value);
	
	PHALCON_INIT_VAR(keys_attributes);
	array_init(keys_attributes);
	
	if (!phalcon_is_iterable(attributes, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
		return;
	}
	
	while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
		PHALCON_GET_FOREACH_VALUE(attribute);
	
		phalcon_array_update_zval(&keys_attributes, attribute, &null_value, PH_COPY | PH_SEPARATE TSRMLS_CC);
	
		zend_hash_move_forward_ex(ah0, &hp0);
	}
	
	PHALCON_INIT_VAR(meta_data);
	PHALCON_CALL_METHOD(meta_data, this_ptr, "getmodelsmetadata");
	PHALCON_CALL_METHOD_PARAMS_2_NORETURN(meta_data, "setautomaticcreateattributes", this_ptr, keys_attributes);
	
	PHALCON_MM_RESTORE();
}

/**
 * Sets a list of attributes that must be skipped from the
 * generated UPDATE statement
 *
 *<code>
 *
 *class Robots extends \Phalcon\Mvc\Model
 *{
 *
 *   public function initialize()
 *   {
 *       $this->skipAttributesOnUpdate(array('modified_in'));
 *   }
 *
 *}
 *</code>
 *
 * @param array $attributes
 */
PHP_METHOD(Phalcon_Mvc_Model, skipAttributesOnUpdate){

	zval *attributes, *null_value, *keys_attributes;
	zval *attribute = NULL, *meta_data;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &attributes) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (Z_TYPE_P(attributes) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Attributes must be an array");
		return;
	}
	
	PHALCON_INIT_VAR(null_value);
	
	PHALCON_INIT_VAR(keys_attributes);
	array_init(keys_attributes);
	
	if (!phalcon_is_iterable(attributes, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
		return;
	}
	
	while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
		PHALCON_GET_FOREACH_VALUE(attribute);
	
		phalcon_array_update_zval(&keys_attributes, attribute, &null_value, PH_COPY | PH_SEPARATE TSRMLS_CC);
	
		zend_hash_move_forward_ex(ah0, &hp0);
	}
	
	PHALCON_INIT_VAR(meta_data);
	PHALCON_CALL_METHOD(meta_data, this_ptr, "getmodelsmetadata");
	PHALCON_CALL_METHOD_PARAMS_2_NORETURN(meta_data, "setautomaticupdateattributes", this_ptr, keys_attributes);
	
	PHALCON_MM_RESTORE();
}

/**
 * Setup a 1-1 relation between two models
 *
 *<code>
 *
 *class Robots extends \Phalcon\Mvc\Model
 *{
 *
 *   public function initialize()
 *   {
 *       $this->hasOne('id', 'RobotsDescription', 'robots_id');
 *   }
 *
 *}
 *</code>
 *
 * @param mixed $fields
 * @param string $referenceModel
 * @param mixed $referencedFields
 * @param   array $options
 */
PHP_METHOD(Phalcon_Mvc_Model, hasOne){

	zval *fields, *reference_model, *referenced_fields;
	zval *options = NULL, *manager;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzz|z", &fields, &reference_model, &referenced_fields, &options) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!options) {
		PHALCON_INIT_VAR(options);
	}
	
	PHALCON_OBS_VAR(manager);
	phalcon_read_property(&manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	PHALCON_CALL_METHOD_PARAMS_5_NORETURN(manager, "addhasone", this_ptr, fields, reference_model, referenced_fields, options);
	
	PHALCON_MM_RESTORE();
}

/**
 * Setup a relation reverse 1-1  between two models
 *
 *<code>
 *
 *class RobotsParts extends \Phalcon\Mvc\Model
 *{
 *
 *   public function initialize()
 *   {
 *       $this->belongsTo('robots_id', 'Robots', 'id');
 *   }
 *
 *}
 *</code>
 *
 * @param mixed $fields
 * @param string $referenceModel
 * @param mixed $referencedFields
 * @param   array $options
 */
PHP_METHOD(Phalcon_Mvc_Model, belongsTo){

	zval *fields, *reference_model, *referenced_fields;
	zval *options = NULL, *manager;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzz|z", &fields, &reference_model, &referenced_fields, &options) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!options) {
		PHALCON_INIT_VAR(options);
	}
	
	PHALCON_OBS_VAR(manager);
	phalcon_read_property(&manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	PHALCON_CALL_METHOD_PARAMS_5_NORETURN(manager, "addbelongsto", this_ptr, fields, reference_model, referenced_fields, options);
	
	PHALCON_MM_RESTORE();
}

/**
 * Setup a relation 1-n between two models
 *
 *<code>
 *
 *class Robots extends \Phalcon\Mvc\Model
 *{
 *
 *   public function initialize()
 *   {
 *       $this->hasMany('id', 'RobotsParts', 'robots_id');
 *   }
 *
 *}
 *</code>
 *
 * @param mixed $fields
 * @param string $referenceModel
 * @param mixed $referencedFields
 * @param   array $options
 */
PHP_METHOD(Phalcon_Mvc_Model, hasMany){

	zval *fields, *reference_model, *referenced_fields;
	zval *options = NULL, *manager;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzz|z", &fields, &reference_model, &referenced_fields, &options) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!options) {
		PHALCON_INIT_VAR(options);
	}
	
	PHALCON_OBS_VAR(manager);
	phalcon_read_property(&manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	PHALCON_CALL_METHOD_PARAMS_5_NORETURN(manager, "addhasmany", this_ptr, fields, reference_model, referenced_fields, options);
	
	PHALCON_MM_RESTORE();
}

/**
 * Setup a relation n-n between two models through an intermediate relation
 *
 *<code>
 *
 *class Robots extends \Phalcon\Mvc\Model
 *{
 *
 *   public function initialize()
 *   {
 *       //A reference relation must be set
 *       $this->hasMany('id', 'RobotsParts', 'robots_id');
 *
 *       //Setup a many-to-many relation to Parts through RobotsParts
 *       $this->hasManyThrough('Parts', 'RobotsParts');
 *   }
 *
 *}
 *</code>
 *
 * @param string $referenceModel
 * @param string $throughRelation
 * @param   array $options
 */
PHP_METHOD(Phalcon_Mvc_Model, hasManyThrough){

	zval *reference_model, *through_relation;
	zval *options = NULL, *manager;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|z", &reference_model, &through_relation, &options) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!options) {
		PHALCON_INIT_VAR(options);
	}
	
	PHALCON_OBS_VAR(manager);
	phalcon_read_property(&manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	PHALCON_CALL_METHOD_PARAMS_3_NORETURN(manager, "addhasmanythrough", this_ptr, reference_model, options);
	
	PHALCON_MM_RESTORE();
}

/**
 * Setups a behavior in a model
 *
 *<code>
 *
 *use Phalcon\Mvc\Model\Behaviors\Timestampable;
 *
 *class Robots extends \Phalcon\Mvc\Model
 *{
 *
 *   public function initialize()
 *   {
 *		$this->addBehavior(new Timestampable(
 *			'onCreate' => array(
 *				'field' => 'created_at',
 *				'format' => 'Y-m-d'
 *			)
 *		));
 *   }
 *
 *}
 *</code>
 *
 * @param Phalcon\Mvc\Model\BehaviorInterface $behavior
 */
PHP_METHOD(Phalcon_Mvc_Model, addBehavior){

	zval *behavior, *manager;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &behavior) == FAILURE) {
		RETURN_MM_NULL();
	}

	PHALCON_OBS_VAR(manager);
	phalcon_read_property(&manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	PHALCON_CALL_METHOD_PARAMS_2_NORETURN(manager, "addbehavior", this_ptr, behavior);
	
	PHALCON_MM_RESTORE();
}

/**
 * Sets if the model must keep the original record snapshot in memory
 *
 *<code>
 *
 *class Robots extends \Phalcon\Mvc\Model
 *{
 *
 *   public function initialize()
 *   {
 *		$this->keepSnapshots(true);
 *   }
 *
 *}
 *</code>
 *
 * @param boolean $keepSnapshots
 */
PHP_METHOD(Phalcon_Mvc_Model, keepSnapshots){

	zval *keep_snapshot, *manager;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &keep_snapshot) == FAILURE) {
		RETURN_MM_NULL();
	}

	PHALCON_OBS_VAR(manager);
	phalcon_read_property(&manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	PHALCON_CALL_METHOD_PARAMS_2_NORETURN(manager, "keepsnapshots", this_ptr, keep_snapshot);
	
	PHALCON_MM_RESTORE();
}

/**
 * Sets the record's snapshot data.
 * This method is used internally to set snapshot data when the model was set up to keep snapshot data
 *
 * @param array $data
 * @param array $columnMap
 */
PHP_METHOD(Phalcon_Mvc_Model, setSnapshotData){

	zval *data, *column_map = NULL, *snapshot, *value = NULL, *key = NULL, *exception_message = NULL;
	zval *attribute = NULL;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z", &data, &column_map) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!column_map) {
		PHALCON_INIT_VAR(column_map);
	}
	
	if (Z_TYPE_P(data) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The snapshot data must be an array");
		return;
	}
	
	/** 
	 * Build the snapshot based on a column map
	 */
	if (Z_TYPE_P(column_map) == IS_ARRAY) { 
	
		PHALCON_INIT_VAR(snapshot);
		array_init(snapshot);
	
		if (!phalcon_is_iterable(data, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
			return;
		}
	
		while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
			PHALCON_GET_FOREACH_KEY(key, ah0, hp0);
			PHALCON_GET_FOREACH_VALUE(value);
	
			if (Z_TYPE_P(key) != IS_STRING) {
				zend_hash_move_forward_ex(ah0, &hp0);
				continue;
			}
	
			/** 
			 * Every field must be part of the column map
			 */
			if (!phalcon_array_isset(column_map, key)) {
				PHALCON_INIT_NVAR(exception_message);
				PHALCON_CONCAT_SVS(exception_message, "Column \"", key, "\" doesn't make part of the column map");
				PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
				return;
			}
	
			PHALCON_OBS_NVAR(attribute);
			phalcon_array_fetch(&attribute, column_map, key, PH_NOISY_CC);
			phalcon_array_update_zval(&snapshot, attribute, &value, PH_COPY | PH_SEPARATE TSRMLS_CC);
	
			zend_hash_move_forward_ex(ah0, &hp0);
		}
	
		phalcon_update_property_zval(this_ptr, SL("_snapshot"), snapshot TSRMLS_CC);
		RETURN_MM_NULL();
	}
	
	phalcon_update_property_zval(this_ptr, SL("_snapshot"), data TSRMLS_CC);
	
	PHALCON_MM_RESTORE();
}

/**
 * Checks if the object has internal snapshot data
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, hasSnapshotData){

	zval *snapshot;

	PHALCON_MM_GROW();

	PHALCON_OBS_VAR(snapshot);
	phalcon_read_property(&snapshot, this_ptr, SL("_snapshot"), PH_NOISY_CC);
	if (Z_TYPE_P(snapshot) == IS_ARRAY) { 
		RETURN_MM_TRUE;
	}
	
	RETURN_MM_FALSE;
}

/**
 * Returns the internal snapshot data
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getSnapshotData){


	RETURN_MEMBER(this_ptr, "_snapshot");
}

/**
 * Check if an specific attribute has changed
 * This only works if the model is keeping data snapshots
 *
 * @param boolean $fieldName
 */
PHP_METHOD(Phalcon_Mvc_Model, hasChanged){

	zval *field_name = NULL, *snapshot, *dirty_state, *meta_data;
	zval *column_map, *attributes, *all_attributes = NULL;
	zval *exception_message = NULL, *value = NULL, *original_value = NULL;
	zval *type = NULL, *name = NULL;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &field_name) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!field_name) {
		PHALCON_INIT_VAR(field_name);
	}
	
	PHALCON_OBS_VAR(snapshot);
	phalcon_read_property(&snapshot, this_ptr, SL("_snapshot"), PH_NOISY_CC);
	if (Z_TYPE_P(snapshot) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The record doesn't have a valid data snapshot");
		return;
	}
	
	if (Z_TYPE_P(field_name) != IS_STRING) {
		if (Z_TYPE_P(field_name) != IS_NULL) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The field name must be string");
			return;
		}
	}
	
	PHALCON_OBS_VAR(dirty_state);
	phalcon_read_property(&dirty_state, this_ptr, SL("_dirtyState"), PH_NOISY_CC);
	
	/** 
	 * Dirty state must be DIRTY_PERSISTENT to make the checking 
	 */
	if (!PHALCON_IS_LONG(dirty_state, 0)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Change checking cannot be performed because the object has not been persisted or is deleted");
		return;
	}
	
	/** 
	 * Return the models meta-data
	 */
	PHALCON_INIT_VAR(meta_data);
	PHALCON_CALL_METHOD(meta_data, this_ptr, "getmodelsmetadata");
	
	/** 
	 * The reversed column map is an array if the model has a column map
	 */
	PHALCON_INIT_VAR(column_map);
	PHALCON_CALL_METHOD_PARAMS_1(column_map, meta_data, "getreversecolumnmap", this_ptr);
	
	/** 
	 * Data types are field indexed
	 */
	if (Z_TYPE_P(column_map) != IS_ARRAY) { 
		PHALCON_INIT_VAR(attributes);
		PHALCON_CALL_METHOD_PARAMS_1(attributes, meta_data, "getdatatypes", this_ptr);
		PHALCON_CPY_WRT(all_attributes, attributes);
	} else {
		PHALCON_CPY_WRT(all_attributes, column_map);
	}
	
	/** 
	 * If a field was specified we only check it
	 */
	if (Z_TYPE_P(field_name) == IS_STRING) {
	
		/** 
		 * We only make this validation over valid fields
		 */
		if (Z_TYPE_P(column_map) == IS_ARRAY) { 
			if (!phalcon_array_isset(column_map, field_name)) {
				PHALCON_INIT_VAR(exception_message);
				PHALCON_CONCAT_SVS(exception_message, "The field '", field_name, "' is not part of the model");
				PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
				return;
			}
		} else {
			if (!phalcon_array_isset(attributes, field_name)) {
				PHALCON_INIT_NVAR(exception_message);
				PHALCON_CONCAT_SVS(exception_message, "The field '", field_name, "' is not part of the model");
				PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
				return;
			}
		}
	
		/** 
		 * The field is not part of the model, throw exception
		 */
		if (!phalcon_isset_property_zval(this_ptr, field_name TSRMLS_CC)) {
			PHALCON_INIT_NVAR(exception_message);
			PHALCON_CONCAT_SVS(exception_message, "The field '", field_name, "' is not defined on the model");
			PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
			return;
		}
	
		/** 
		 * The field is not part of the data snapshot, throw exception
		 */
		if (!phalcon_array_isset(snapshot, field_name)) {
			PHALCON_INIT_NVAR(exception_message);
			PHALCON_CONCAT_SVS(exception_message, "The field '", field_name, "' was not found in the snapshot");
			PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
			return;
		}
	
		PHALCON_OBS_VAR(value);
		phalcon_read_property_zval(&value, this_ptr, field_name, PH_NOISY_CC);
	
		PHALCON_OBS_VAR(original_value);
		phalcon_array_fetch(&original_value, snapshot, field_name, PH_NOISY_CC);
	
		/** 
		 * Check if the field has changed
		 */
		if (PHALCON_IS_EQUAL(value, original_value)) {
			RETURN_MM_FALSE;
		} else {
			RETURN_MM_TRUE;
		}
	}
	
	/** 
	 * Check every attribute in the model
	 */
	
	if (!phalcon_is_iterable(all_attributes, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
		return;
	}
	
	while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
		PHALCON_GET_FOREACH_KEY(name, ah0, hp0);
		PHALCON_GET_FOREACH_VALUE(type);
	
		/** 
		 * If some attribute is not present in the snapshot, we assume the record as
		 * changed
		 */
		if (!phalcon_array_isset(snapshot, name)) {
			RETURN_MM_TRUE;
		}
	
		/** 
		 * If some attribute is not present in the model, we assume the record as changed
		 */
		if (!phalcon_isset_property_zval(this_ptr, name TSRMLS_CC)) {
			RETURN_MM_TRUE;
		}
	
		PHALCON_OBS_NVAR(value);
		phalcon_read_property_zval(&value, this_ptr, name, PH_NOISY_CC);
	
		PHALCON_OBS_NVAR(original_value);
		phalcon_array_fetch(&original_value, snapshot, name, PH_NOISY_CC);
	
		/** 
		 * Check if the field has changed
		 */
		if (!PHALCON_IS_EQUAL(value, original_value)) {
			RETURN_MM_TRUE;
		}
	
		zend_hash_move_forward_ex(ah0, &hp0);
	}
	
	RETURN_MM_FALSE;
}

/**
 * Returns a list of changed values
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getChangedFields){

	zval *snapshot, *dirty_state, *meta_data, *column_map;
	zval *attributes, *all_attributes = NULL, *changed;
	zval *type = NULL, *name = NULL, *value = NULL, *original_value = NULL;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	PHALCON_OBS_VAR(snapshot);
	phalcon_read_property(&snapshot, this_ptr, SL("_snapshot"), PH_NOISY_CC);
	if (Z_TYPE_P(snapshot) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The record doesn't have a valid data snapshot");
		return;
	}
	
	PHALCON_OBS_VAR(dirty_state);
	phalcon_read_property(&dirty_state, this_ptr, SL("_dirtyState"), PH_NOISY_CC);
	
	/** 
	 * Dirty state must be DIRTY_PERSISTENT to make the checking 
	 */
	if (!PHALCON_IS_LONG(dirty_state, 0)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Change checking cannot be performed because the object has not been persisted or is deleted");
		return;
	}
	
	/** 
	 * Return the models meta-data
	 */
	PHALCON_INIT_VAR(meta_data);
	PHALCON_CALL_METHOD(meta_data, this_ptr, "getmodelsmetadata");
	
	/** 
	 * The reversed column map is an array if the model has a column map
	 */
	PHALCON_INIT_VAR(column_map);
	PHALCON_CALL_METHOD_PARAMS_1(column_map, meta_data, "getreversecolumnmap", this_ptr);
	
	/** 
	 * Data types are field indexed
	 */
	if (Z_TYPE_P(column_map) != IS_ARRAY) { 
		PHALCON_INIT_VAR(attributes);
		PHALCON_CALL_METHOD_PARAMS_1(attributes, meta_data, "getdatatypes", this_ptr);
		PHALCON_CPY_WRT(all_attributes, attributes);
	} else {
		PHALCON_CPY_WRT(all_attributes, column_map);
	}
	
	PHALCON_INIT_VAR(changed);
	array_init(changed);
	
	/** 
	 * Check every attribute in the model
	 */
	
	if (!phalcon_is_iterable(all_attributes, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
		return;
	}
	
	while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
		PHALCON_GET_FOREACH_KEY(name, ah0, hp0);
		PHALCON_GET_FOREACH_VALUE(type);
	
		/** 
		 * If some attribute is not present in the snapshot, we assume the record as
		 * changed
		 */
		if (!phalcon_array_isset(snapshot, name)) {
			phalcon_array_append(&changed, name, PH_SEPARATE TSRMLS_CC);
			zend_hash_move_forward_ex(ah0, &hp0);
			continue;
		}
	
		/** 
		 * If some attribute is not present in the model, we assume the record as changed
		 */
		if (!phalcon_isset_property_zval(this_ptr, name TSRMLS_CC)) {
			phalcon_array_append(&changed, name, PH_SEPARATE TSRMLS_CC);
			zend_hash_move_forward_ex(ah0, &hp0);
			continue;
		}
	
		PHALCON_OBS_NVAR(value);
		phalcon_read_property_zval(&value, this_ptr, name, PH_NOISY_CC);
	
		PHALCON_OBS_NVAR(original_value);
		phalcon_array_fetch(&original_value, snapshot, name, PH_NOISY_CC);
	
		/** 
		 * Check if the field has changed
		 */
		if (!PHALCON_IS_EQUAL(value, original_value)) {
			phalcon_array_append(&changed, name, PH_SEPARATE TSRMLS_CC);
			zend_hash_move_forward_ex(ah0, &hp0);
			continue;
		}
	
		zend_hash_move_forward_ex(ah0, &hp0);
	}
	
	
	RETURN_CTOR(changed);
}

/**
 * Sets if a model must use dynamic update instead of the all-field update
 *
 *<code>
 *
 *class Robots extends \Phalcon\Mvc\Model
 *{
 *
 *   public function initialize()
 *   {
 *		$this->useDynamicUpdate(true);
 *   }
 *
 *}
 *</code>
 *
 * @param boolean $dynamicUpdate
 */
PHP_METHOD(Phalcon_Mvc_Model, useDynamicUpdate){

	zval *dynamic_update, *manager;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &dynamic_update) == FAILURE) {
		RETURN_MM_NULL();
	}

	PHALCON_OBS_VAR(manager);
	phalcon_read_property(&manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	PHALCON_CALL_METHOD_PARAMS_2_NORETURN(manager, "usedynamicupdate", this_ptr, dynamic_update);
	
	PHALCON_MM_RESTORE();
}

/**
 * Returns related records based on defined relations
 *
 * @param string $alias
 * @param array $arguments
 * @return Phalcon\Mvc\Model\ResultsetInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, getRelated){

	zval *alias, *arguments = NULL, *manager, *class_name;
	zval *relation, *exception_message, *call_object;
	zval *model_args, *result;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z", &alias, &arguments) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!arguments) {
		PHALCON_INIT_VAR(arguments);
	}
	
	PHALCON_OBS_VAR(manager);
	phalcon_read_property(&manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	
	PHALCON_INIT_VAR(class_name);
	phalcon_get_class(class_name, this_ptr, 0 TSRMLS_CC);
	
	/** 
	 * Query the relation by alias
	 */
	PHALCON_INIT_VAR(relation);
	PHALCON_CALL_METHOD_PARAMS_2(relation, manager, "getrelationbyalias", class_name, alias);
	if (Z_TYPE_P(relation) != IS_OBJECT) {
		PHALCON_INIT_VAR(exception_message);
		PHALCON_CONCAT_SVSVS(exception_message, "There is no defined relations for the model \"", class_name, "\" using alias \"", alias, "\"");
		PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
		return;
	}
	
	/** 
	 * Call the 'getRelationRecords' in the models manager
	 */
	PHALCON_INIT_VAR(call_object);
	array_init_size(call_object, 2);
	phalcon_array_append(&call_object, manager, PH_SEPARATE TSRMLS_CC);
	add_next_index_stringl(call_object, SL("getRelationRecords"), 1);
	
	PHALCON_INIT_VAR(model_args);
	array_init_size(model_args, 4);
	phalcon_array_append(&model_args, relation, PH_SEPARATE TSRMLS_CC);
	add_next_index_null(model_args);
	phalcon_array_append(&model_args, this_ptr, PH_SEPARATE TSRMLS_CC);
	phalcon_array_append(&model_args, arguments, PH_SEPARATE TSRMLS_CC);
	
	PHALCON_INIT_VAR(result);
	PHALCON_CALL_USER_FUNC_ARRAY(result, call_object, model_args);
	
	RETURN_CCTOR(result);
}

/**
 * Returns related records defined relations depending on the method name
 *
 * @param string $modelName
 * @param string $method
 * @param array $arguments
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Model, _getRelatedRecords){

	zval *model_name, *method, *arguments, *manager;
	zval *relation = NULL, *query_method = NULL, *alias = NULL, *extra_args = NULL;
	zval *call_args, *call_object, *result;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzz", &model_name, &method, &arguments) == FAILURE) {
		RETURN_MM_NULL();
	}

	PHALCON_OBS_VAR(manager);
	phalcon_read_property(&manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	
	PHALCON_INIT_VAR(relation);
	ZVAL_BOOL(relation, 0);
	
	PHALCON_INIT_VAR(query_method);
	
	/** 
	 * Calling find/findFirst if the method starts with "get"
	 */
	if (phalcon_start_with_str(method, SL("get"))) {
		PHALCON_INIT_VAR(alias);
		phalcon_substr(alias, method, 3, 0 TSRMLS_CC);
	
		PHALCON_CALL_METHOD_PARAMS_2(relation, manager, "getrelationbyalias", model_name, alias);
	}
	
	/** 
	 * Calling count if the method starts with "count"
	 */
	if (Z_TYPE_P(relation) != IS_OBJECT) {
		if (phalcon_start_with_str(method, SL("count"))) {
			ZVAL_STRING(query_method, "count", 1);
	
			PHALCON_INIT_NVAR(alias);
			phalcon_substr(alias, method, 5, 0 TSRMLS_CC);
	
			PHALCON_INIT_NVAR(relation);
			PHALCON_CALL_METHOD_PARAMS_2(relation, manager, "getrelationbyalias", model_name, alias);
		}
	}
	
	/** 
	 * If the relation was found perform the query via the models manager
	 */
	if (Z_TYPE_P(relation) == IS_OBJECT) {
		if (phalcon_array_isset_long(arguments, 0)) {
			PHALCON_OBS_VAR(extra_args);
			phalcon_array_fetch_long(&extra_args, arguments, 0, PH_NOISY_CC);
		} else {
			PHALCON_INIT_NVAR(extra_args);
		}
	
		PHALCON_INIT_VAR(call_args);
		array_init_size(call_args, 4);
		phalcon_array_append(&call_args, relation, PH_SEPARATE TSRMLS_CC);
		phalcon_array_append(&call_args, query_method, PH_SEPARATE TSRMLS_CC);
		phalcon_array_append(&call_args, this_ptr, PH_SEPARATE TSRMLS_CC);
		phalcon_array_append(&call_args, extra_args, PH_SEPARATE TSRMLS_CC);
	
		PHALCON_INIT_VAR(call_object);
		array_init_size(call_object, 2);
		phalcon_array_append(&call_object, manager, PH_SEPARATE TSRMLS_CC);
		add_next_index_stringl(call_object, SL("getRelationRecords"), 1);
	
		PHALCON_INIT_VAR(result);
		PHALCON_CALL_USER_FUNC_ARRAY(result, call_object, call_args);
	
		RETURN_CCTOR(result);
	}
	
	RETURN_MM_NULL();
}

/**
 * Handles method calls when a method is not implemented
 *
 * @param string $method
 * @param array $arguments
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Model, __call){

	zval *method, *arguments = NULL, *model_name, *records;
	zval *models_manager, *status, *exception_message;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z", &method, &arguments) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!arguments) {
		PHALCON_INIT_VAR(arguments);
		array_init(arguments);
	}
	
	PHALCON_INIT_VAR(model_name);
	phalcon_get_class(model_name, this_ptr, 0 TSRMLS_CC);
	
	/** 
	 * Check if there is a default action using the magic getter
	 */
	PHALCON_INIT_VAR(records);
	PHALCON_CALL_METHOD_PARAMS_3(records, this_ptr, "_getrelatedrecords", model_name, method, arguments);
	if (Z_TYPE_P(records) != IS_NULL) {
		RETURN_CCTOR(records);
	}
	
	PHALCON_OBS_VAR(models_manager);
	phalcon_read_property(&models_manager, this_ptr, SL("_modelsManager"), PH_NOISY_CC);
	
	/** 
	 * Try to find a replacement for the missing method in a behavior/listener
	 */
	PHALCON_INIT_VAR(status);
	PHALCON_CALL_METHOD_PARAMS_3(status, models_manager, "missingmethod", this_ptr, method, arguments);
	if (Z_TYPE_P(status) != IS_NULL) {
		RETURN_CCTOR(status);
	}
	
	/** 
	 * The method doesn't exist throw an exception
	 */
	PHALCON_INIT_VAR(exception_message);
	PHALCON_CONCAT_SVSVS(exception_message, "The method \"", method, "\" doesn't exist on model \"", model_name, "\"");
	PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
	return;
}

/**
 * Handles method calls when a static method is not implemented
 *
 * @param string $method
 * @param array $arguments
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Model, __callStatic){

	zval *method, *arguments = NULL, *extra_method = NULL, *type = NULL;
	zval *model_name, *exception_message = NULL, *value;
	zval *uncamelized, *conditions, *bind_params;
	zval *parameters, *result;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z", &method, &arguments) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (!arguments) {
		PHALCON_INIT_VAR(arguments);
	}
	
	PHALCON_INIT_VAR(extra_method);
	
	/** 
	 * Check if the method starts with 'findFirst'
	 */
	if (phalcon_start_with_str(method, SL("findFirstBy"))) {
		PHALCON_INIT_VAR(type);
		ZVAL_STRING(type, "findFirst", 1);
	
		phalcon_substr(extra_method, method, 11, 0 TSRMLS_CC);
	}
	
	/** 
	 * Check if the method starts with 'find'
	 */
	if (Z_TYPE_P(extra_method) == IS_NULL) {
		if (phalcon_start_with_str(method, SL("findBy"))) {
			PHALCON_INIT_NVAR(type);
			ZVAL_STRING(type, "find", 1);
	
			PHALCON_INIT_NVAR(extra_method);
			phalcon_substr(extra_method, method, 6, 0 TSRMLS_CC);
		}
	}
	
	/** 
	 * Check if the method starts with 'count'
	 */
	if (Z_TYPE_P(extra_method) == IS_NULL) {
		if (phalcon_start_with_str(method, SL("countBy"))) {
			PHALCON_INIT_NVAR(type);
			ZVAL_STRING(type, "count", 1);
	
			PHALCON_INIT_NVAR(extra_method);
			phalcon_substr(extra_method, method, 7, 0 TSRMLS_CC);
		}
	}
	
	/** 
	 * The called class is the model
	 */
	PHALCON_INIT_VAR(model_name);
	phalcon_get_called_class(model_name  TSRMLS_CC);
	if (!zend_is_true(extra_method)) {
		/** 
		 * The method doesn't exist throw an exception
		 */
		PHALCON_INIT_VAR(exception_message);
		PHALCON_CONCAT_SVSVS(exception_message, "The static method \"", method, "\" doesn't exist on model \"", model_name, "\"");
		PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
		return;
	}
	
	if (!phalcon_array_isset_long(arguments, 0)) {
		PHALCON_INIT_NVAR(exception_message);
		PHALCON_CONCAT_SVS(exception_message, "The static method \"", method, "\" requires one argument");
		PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
		return;
	}
	
	PHALCON_OBS_VAR(value);
	phalcon_array_fetch_long(&value, arguments, 0, PH_NOISY_CC);
	
	/** 
	 * Get the real method name
	 */
	PHALCON_INIT_VAR(uncamelized);
	phalcon_uncamelize(uncamelized, extra_method TSRMLS_CC);
	
	PHALCON_INIT_VAR(conditions);
	PHALCON_CONCAT_VS(conditions, uncamelized, " = ?0");
	
	PHALCON_INIT_VAR(bind_params);
	array_init_size(bind_params, 1);
	phalcon_array_append(&bind_params, value, PH_SEPARATE TSRMLS_CC);
	
	PHALCON_INIT_VAR(parameters);
	array_init_size(parameters, 2);
	phalcon_array_update_string(&parameters, SL("conditions"), &conditions, PH_COPY | PH_SEPARATE TSRMLS_CC);
	phalcon_array_update_string(&parameters, SL("bind"), &bind_params, PH_COPY | PH_SEPARATE TSRMLS_CC);
	
	/** 
	 * Execute the query
	 */
	PHALCON_INIT_VAR(result);
	PHALCON_CALL_STATIC_ZVAL_PARAMS_1(result, model_name, Z_STRVAL_P(type), parameters);
	
	RETURN_CCTOR(result);
}

/**
 * Magic method to assign values to the the model
 *
 * @param string $property
 * @param mixed $value
 */
PHP_METHOD(Phalcon_Mvc_Model, __set){

	zval *property, *value, *is_model, *lower_property = NULL;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &property, &value) == FAILURE) {
		RETURN_MM_NULL();
	}

	/** 
	 * Values are probably relationships if they are objects
	 */
	if (Z_TYPE_P(value) == IS_OBJECT) {
	
		PHALCON_INIT_VAR(is_model);
		phalcon_instance_of(is_model, value, phalcon_mvc_modelinterface_ce TSRMLS_CC);
		if (zend_is_true(is_model)) {
			PHALCON_INIT_VAR(lower_property);
			phalcon_fast_strtolower(lower_property, property);
			phalcon_update_property_zval_zval(this_ptr, lower_property, value TSRMLS_CC);
			phalcon_update_property_array(this_ptr, SL("_related"), lower_property, value TSRMLS_CC);
			phalcon_update_property_long(this_ptr, SL("_dirtyState"), 1 TSRMLS_CC);
			RETURN_CCTOR(value);
		}
	}
	
	/** 
	 * Check if the value is an array
	 */
	if (Z_TYPE_P(value) == IS_ARRAY) { 
		PHALCON_INIT_NVAR(lower_property);
		phalcon_fast_strtolower(lower_property, property);
		phalcon_update_property_array(this_ptr, SL("_related"), lower_property, value TSRMLS_CC);
		phalcon_update_property_long(this_ptr, SL("_dirtyState"), 1 TSRMLS_CC);
		RETURN_CCTOR(value);
	}
	
	/** 
	 * Fallback assigning the value to the instance
	 */
	phalcon_update_property_zval_zval(this_ptr, property, value TSRMLS_CC);
	
	RETURN_CCTOR(value);
}

/**
 * Magic method to get related records using the relation alias as a property
 *
 * @param string $property
 * @return Phalcon\Mvc\Model\Resultset
 */
PHP_METHOD(Phalcon_Mvc_Model, __get){

	zval *property, *model_name, *manager, *lower_property;
	zval *relation, *call_args, *call_object, *result;
	zval *is_simple_model, *error_msg;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &property) == FAILURE) {
		RETURN_MM_NULL();
	}

	PHALCON_INIT_VAR(model_name);
	phalcon_get_class(model_name, this_ptr, 0 TSRMLS_CC);
	
	PHALCON_INIT_VAR(manager);
	PHALCON_CALL_METHOD(manager, this_ptr, "getmodelsmanager");
	
	PHALCON_INIT_VAR(lower_property);
	phalcon_fast_strtolower(lower_property, property);
	
	/** 
	 * Check if the property is a relationship
	 */
	PHALCON_INIT_VAR(relation);
	PHALCON_CALL_METHOD_PARAMS_2(relation, manager, "getrelationbyalias", model_name, lower_property);
	if (Z_TYPE_P(relation) == IS_OBJECT) {
	
		PHALCON_INIT_VAR(call_args);
		array_init_size(call_args, 4);
		phalcon_array_append(&call_args, relation, PH_SEPARATE TSRMLS_CC);
		add_next_index_null(call_args);
		phalcon_array_append(&call_args, this_ptr, PH_SEPARATE TSRMLS_CC);
		add_next_index_null(call_args);
	
		PHALCON_INIT_VAR(call_object);
		array_init_size(call_object, 2);
		phalcon_array_append(&call_object, manager, PH_SEPARATE TSRMLS_CC);
		add_next_index_stringl(call_object, SL("getRelationRecords"), 1);
	
		/** 
		 * Get the related records
		 */
		PHALCON_INIT_VAR(result);
		PHALCON_CALL_USER_FUNC_ARRAY(result, call_object, call_args);
	
		/** 
		 * Assign the result to the object
		 */
		if (Z_TYPE_P(result) == IS_OBJECT) {
	
			/** 
			 * We assign the result to the instance avoiding future queries
			 */
			phalcon_update_property_zval_zval(this_ptr, lower_property, result TSRMLS_CC);
	
			/** 
			 * For belongs-to relations we store the object in the related bag
			 */
			PHALCON_INIT_VAR(is_simple_model);
			phalcon_instance_of(is_simple_model, result, phalcon_mvc_modelinterface_ce TSRMLS_CC);
			if (PHALCON_IS_TRUE(is_simple_model)) {
				phalcon_update_property_array(this_ptr, SL("_related"), lower_property, result TSRMLS_CC);
			}
		}
	
	
		RETURN_CCTOR(result);
	}
	
	/** 
	 * A notice is shown if the property is not defined and it isn't a relationship
	 */
	PHALCON_INIT_VAR(error_msg);
	PHALCON_CONCAT_SVSV(error_msg, "Access to undefined property ", model_name, "::", property);
	PHALCON_CALL_FUNC_PARAMS_1_NORETURN("trigger_error", error_msg);
	RETURN_MM_NULL();
}

/**
 * Magic method to check if a property is a valid relation
 *
 * @param string $property
 */
PHP_METHOD(Phalcon_Mvc_Model, __isset){

	zval *property, *model_name, *manager, *relation;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &property) == FAILURE) {
		RETURN_MM_NULL();
	}

	PHALCON_INIT_VAR(model_name);
	phalcon_get_class(model_name, this_ptr, 0 TSRMLS_CC);
	
	PHALCON_INIT_VAR(manager);
	PHALCON_CALL_METHOD(manager, this_ptr, "getmodelsmanager");
	
	/** 
	 * Check if the property is a relationship
	 */
	PHALCON_INIT_VAR(relation);
	PHALCON_CALL_METHOD_PARAMS_2(relation, manager, "getrelationbyalias", model_name, property);
	if (Z_TYPE_P(relation) == IS_OBJECT) {
		RETURN_MM_TRUE;
	}
	
	RETURN_MM_FALSE;
}

/**
 * Serializes the object ignoring connections or static properties
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model, serialize){

	zval *meta_data, *attributes, *null_value, *data;
	zval *attribute = NULL, *value = NULL, *serialize;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	PHALCON_INIT_VAR(meta_data);
	PHALCON_CALL_METHOD(meta_data, this_ptr, "getmodelsmetadata");
	
	/** 
	 * We get the model's attributes to only serialize them
	 */
	PHALCON_INIT_VAR(attributes);
	PHALCON_CALL_METHOD_PARAMS_1(attributes, meta_data, "getattributes", this_ptr);
	
	PHALCON_INIT_VAR(null_value);
	
	PHALCON_INIT_VAR(data);
	array_init(data);
	
	if (!phalcon_is_iterable(attributes, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
		return;
	}
	
	while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
		PHALCON_GET_FOREACH_VALUE(attribute);
	
		if (phalcon_isset_property_zval(this_ptr, attribute TSRMLS_CC)) {
			PHALCON_OBS_NVAR(value);
			phalcon_read_property_zval(&value, this_ptr, attribute, PH_NOISY_CC);
			phalcon_array_update_zval(&data, attribute, &value, PH_COPY | PH_SEPARATE TSRMLS_CC);
		} else {
			phalcon_array_update_zval(&data, attribute, &null_value, PH_COPY | PH_SEPARATE TSRMLS_CC);
		}
	
		zend_hash_move_forward_ex(ah0, &hp0);
	}
	
	/** 
	 * Use the standard serialize function to serialize the array data
	 */
	PHALCON_INIT_VAR(serialize);
	PHALCON_CALL_FUNC_PARAMS_1(serialize, "serialize", data);
	
	RETURN_CCTOR(serialize);
}

/**
 * Unserializes the object from a serialized string
 *
 * @param string $data
 */
PHP_METHOD(Phalcon_Mvc_Model, unserialize){

	zval *data, *attributes, *dependency_injector;
	zval *service, *manager, *value = NULL, *key = NULL;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &data) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (Z_TYPE_P(data) == IS_STRING) {
	
		PHALCON_INIT_VAR(attributes);
		PHALCON_CALL_FUNC_PARAMS_1(attributes, "unserialize", data);
		if (Z_TYPE_P(attributes) == IS_ARRAY) { 
	
			/** 
			 * Obtain the default DI
			 */
			PHALCON_INIT_VAR(dependency_injector);
			PHALCON_CALL_STATIC(dependency_injector, "phalcon\\di", "getdefault");
			if (Z_TYPE_P(dependency_injector) != IS_OBJECT) {
				PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "A dependency injector container is required to obtain the services related to the ORM");
				return;
			}
	
			/** 
			 * Update the dependency injector
			 */
			phalcon_update_property_zval(this_ptr, SL("_dependencyInjector"), dependency_injector TSRMLS_CC);
	
			/** 
			 * Gets the default modelsManager service
			 */
			PHALCON_INIT_VAR(service);
			ZVAL_STRING(service, "modelsManager", 1);
	
			PHALCON_INIT_VAR(manager);
			PHALCON_CALL_METHOD_PARAMS_1(manager, dependency_injector, "getshared", service);
			if (Z_TYPE_P(manager) != IS_OBJECT) {
				PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The injected service 'modelsManager' is not valid");
				return;
			}
	
			/** 
			 * Update the models manager
			 */
			phalcon_update_property_zval(this_ptr, SL("_modelsManager"), manager TSRMLS_CC);
	
			/** 
			 * Try to initialize the model
			 */
			PHALCON_CALL_METHOD_PARAMS_1_NORETURN(manager, "initialize", this_ptr);
	
			/** 
			 * Update the objects attributes
			 */
	
			if (!phalcon_is_iterable(attributes, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
				return;
			}
	
			while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
				PHALCON_GET_FOREACH_KEY(key, ah0, hp0);
				PHALCON_GET_FOREACH_VALUE(value);
	
				phalcon_update_property_zval_zval(this_ptr, key, value TSRMLS_CC);
	
				zend_hash_move_forward_ex(ah0, &hp0);
			}
	
			RETURN_MM_NULL();
		}
	}
	PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Invalid serialization data");
	return;
}

/**
 * Returns a simple representation of the object that can be used with var_dump
 *
 *<code>
 * var_dump($robot->dump());
 *</code>
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, dump){

	zval *array_data;

	PHALCON_MM_GROW();

	PHALCON_INIT_VAR(array_data);
	PHALCON_CALL_FUNC_PARAMS_1(array_data, "get_object_vars", this_ptr);
	RETURN_CCTOR(array_data);
}

/**
 * Returns the instance as an array representation
 *
 *<code>
 * print_r($robot->toArray());
 *</code>
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, toArray){

	zval *meta_data, *data, *null_value, *attributes;
	zval *column_map, *attribute = NULL, *exception_message = NULL;
	zval *attribute_field = NULL, *value = NULL;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	PHALCON_INIT_VAR(meta_data);
	PHALCON_CALL_METHOD(meta_data, this_ptr, "getmodelsmetadata");
	
	PHALCON_INIT_VAR(data);
	array_init(data);
	
	PHALCON_INIT_VAR(null_value);
	
	/** 
	 * Original attributes
	 */
	PHALCON_INIT_VAR(attributes);
	PHALCON_CALL_METHOD_PARAMS_1(attributes, meta_data, "getattributes", this_ptr);
	
	/** 
	 * Reverse column map
	 */
	PHALCON_INIT_VAR(column_map);
	PHALCON_CALL_METHOD_PARAMS_1(column_map, meta_data, "getcolumnmap", this_ptr);
	
	if (!phalcon_is_iterable(attributes, &ah0, &hp0, 0, 0 TSRMLS_CC)) {
		return;
	}
	
	while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
	
		PHALCON_GET_FOREACH_VALUE(attribute);
	
		/** 
		 * Check if the columns must be renamed
		 */
		if (Z_TYPE_P(column_map) == IS_ARRAY) { 
			if (!phalcon_array_isset(column_map, attribute)) {
				PHALCON_INIT_NVAR(exception_message);
				PHALCON_CONCAT_SVS(exception_message, "Column \"", attribute, "\" doesn't make part of the column map");
				PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
				return;
			}
	
			PHALCON_OBS_NVAR(attribute_field);
			phalcon_array_fetch(&attribute_field, column_map, attribute, PH_NOISY_CC);
		} else {
			PHALCON_CPY_WRT(attribute_field, attribute);
		}
		if (phalcon_isset_property_zval(this_ptr, attribute_field TSRMLS_CC)) {
			PHALCON_OBS_NVAR(value);
			phalcon_read_property_zval(&value, this_ptr, attribute_field, PH_NOISY_CC);
			phalcon_array_update_zval(&data, attribute_field, &value, PH_COPY | PH_SEPARATE TSRMLS_CC);
		} else {
			phalcon_array_update_zval(&data, attribute_field, &null_value, PH_COPY | PH_SEPARATE TSRMLS_CC);
		}
	
		zend_hash_move_forward_ex(ah0, &hp0);
	}
	
	
	RETURN_CTOR(data);
}

/**
 * Enables/disables options in the ORM
 *
 * @param array $options
 */
PHP_METHOD(Phalcon_Mvc_Model, setup){

	zval *options, *disable_events, *virtual_foreign_keys;
	zval *column_renaming, *not_null_validations;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &options) == FAILURE) {
		RETURN_MM_NULL();
	}

	if (Z_TYPE_P(options) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Options must be an array");
		return;
	}
	
	/** 
	 * Enables/Disables globally the internal events
	 */
	if (phalcon_array_isset_string(options, SS("events"))) {
		PHALCON_OBS_VAR(disable_events);
		phalcon_array_fetch_string(&disable_events, options, SL("events"), PH_NOISY_CC);
		PHALCON_GLOBAL(orm).events = zend_is_true(disable_events);
	}
	
	/** 
	 * Enables/Disables virtual foreign keys
	 */
	if (phalcon_array_isset_string(options, SS("virtualForeignKeys"))) {
		PHALCON_OBS_VAR(virtual_foreign_keys);
		phalcon_array_fetch_string(&virtual_foreign_keys, options, SL("virtualForeignKeys"), PH_NOISY_CC);
		PHALCON_GLOBAL(orm).virtual_foreign_keys = zend_is_true(virtual_foreign_keys);
	}
	
	/** 
	 * Enables/Disables column renaming
	 */
	if (phalcon_array_isset_string(options, SS("columnRenaming"))) {
		PHALCON_OBS_VAR(column_renaming);
		phalcon_array_fetch_string(&column_renaming, options, SL("columnRenaming"), PH_NOISY_CC);
		PHALCON_GLOBAL(orm).column_renaming = zend_is_true(column_renaming);
	}
	
	/** 
	 * Enables/Disables automatic not null validation
	 */
	if (phalcon_array_isset_string(options, SS("notNullValidations"))) {
		PHALCON_OBS_VAR(not_null_validations);
		phalcon_array_fetch_string(&not_null_validations, options, SL("notNullValidations"), PH_NOISY_CC);
		PHALCON_GLOBAL(orm).not_null_validations = zend_is_true(not_null_validations);
	}
	
	PHALCON_MM_RESTORE();
}

