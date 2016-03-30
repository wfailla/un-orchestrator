#!/usr/bin/python3
# -*- coding: utf-8 -*-
import argparse
import logging
from doubledecker.clientSafe import ClientSafe
import json
import sys
import os
from collections import OrderedDict
from functools import reduce

# For PipelineDB integration
import psycopg2

import pdb

# This program aggregates rate monitor data from several RAMON
# instances.
#
# Accepts the Json-Rpc notifications 'rate_data', 'add_monitor' and
# 'remove_monitor' over the DoubleDecker bus.
# {'jsonrpc': "2.0", 'method': 'rate_data', 'params': {'name': <monitor name>, 'lm: <location value> ,'lsd': <scale value>):
# {'jsonrpc': "2.0", 'method': 'add_monitor', 'params': {'name': <monitor name> }}
# {'jsonrpc': "2.0", 'method': 'remove_monitor', 'params': {'name': <monitor name> }}
# {'jsonrpc': "2.0", 'method': 'set_alarm_level', 'params': {'alarm_level': <level> }}
#
# In case of an overload risk the aggregator will publish the
# following Json-Rpc notification on the topic 'monitor_alarm' on the
# DoubleDecker bus:
# {"jsonrpc": "2.0", "method": "overload_alarm", "params": {"overload_risk": <risk_value>}}
#
# Startup.
#
# If started from a command line you can set startup parameters via
# options. Do 'aggregator.py --help' for a list of startup parameters.
#
# If the environment variables PIPELINEDB_PORT_5432_TCP_ADDR and/or
# PIPELINEDB_PORT_5432_TCP_PORT are set, they are used if the
# corresponding startup options are not set.


# Keeps track of the data sent by the monitors
class RateMonitors():

    # This init method will probably never be used.
    # add_monitor() is more flexible.
    def __init__(self, monitors = {"m1", "m2", "m3", "m4"}):
        self.monitors = {}
        for m in monitors:
            self.monitors[m] = False
        logging.debug('RateMonitors is initialized')

    # Returns True if all monitors have sent their rate data
    def is_complete(self):
        result = reduce(lambda x,y: x and y, list(self.monitors.values()))
        logging.debug('RateMonitors.is_complete() returning ' + str(result))
        logging.debug('RateMonitors.monitors==%s'%self.monitors)
        return result

    # Mark a monitor as having sent its rate data
    def set_monitor_complete(self,name):
        self.monitors[name] = True
        logging.debug('RateMonitors.set_monitor_complete(%s)'%name)
        logging.debug('RateMonitors.monitors[%s]==%s'%(name,self.monitors[name]))

    # Set all monitors to incomplete
    def reset(self):
        for m in self.monitors:
            self.monitors[m] = False
        logging.debug('RateMonitors.reset()')

    def add_monitor(self,name):
        self.monitors[name] = False
        logging.debug('RateMonitors.add_monitor(%s)'%name)

    def remove_monitor(self,name):
        self.monitors.pop(name, None)
        logging.debug('RateMonitors.remove_monitor(%s)'%name)

    def get_monitor_names(self):
        return tuple(self.monitors.keys())

class Aggregator(ClientSafe):
    def __init__(self, name, dealerurl, customer, keyfile,
                 topics = ['monitor_aggregate/node'],
                 dbname = "pipeline",
                 dbuser = "pipeline",
                 dbpass = "pipeline",
                 dbhost = "127.0.0.1",
                 dbport = "5432",
                 alarm_level = 0.9):
        super().__init__(name, dealerurl, customer, keyfile)
        self.mytopics = list()

        try:
            for top in topics.split(","):
                if len(top) == 0:
                    return
                (topic,scope) = top.split("/")
                self.mytopics.append((topic,scope))
        except ValueError:
            logging.error("Could not parse topics!")
            sys.exit(1)

        connstr = 'dbname=' + dbname + ' user= ' + dbuser + ' password=' + dbpass + ' host=' + dbhost + ' port=' + dbport
        logging.info("Aggregator: attempting connection to " + connstr)
        self.conn = psycopg2.connect(connstr)
        self.pipeline = self.conn.cursor()
#        self.drop_all_functions()
        self.create_all_functions()
        self.rates = RateMonitors({})
        self.alarm_level = alarm_level
    
    def on_data(self, src, rpc):
        msg = dict()
        msg["type"] = "data"
        msg["src"] = src.decode()
        msg["data"] = data.decode()
        print(json.dumps(msg))

    def rate_data(self,params):
        lm = params['lm']
        lsd = params['lsd']
        name = params['name']
        self.insert_into_stream(name,lm,lsd)
        self.rates.set_monitor_complete(name)
        if self.rates.is_complete():
            self.evaluate_overload_risk()
            self.rates.reset()

    def add_monitor(self,params):
        name = params["name"]
        self.create_stream(name)
        self.setup_continuous_view(name)
        self.rates.add_monitor(name)

    def remove_monitor(self,params):
        self.remove_continuous_view(params["name"])

    def set_alarm_level(self,params):
        self.alarm_level = float(params["alarm_level"])
        

    def evaluate_overload_risk(self):
        print('evaluate_overload_risk called!')
        alarm_sql = "select lognormagg(ARRAY[lm,lsd]) from stream_%s,stream_%s,stream_%s,stream_%s"%Rates.get_monitor_names()
        perform_sql_with_commit(alarm_sql)
        try:
            rows = curs.fetchall()
            for row in rows:
                logging.info('perform_sql, result: ' + str(row))
        except psycopg2.Error as e:
            logging.warning('perform_sql, result error: ' + str(e))
            zmu = rows[0][0]
            zsd = rows[0][1]
            ## Using the survival function (1 - cdf). See http://docs.scipy.org/doc/scipy/reference/generated/scipy.stats.lognorm.html for a motivation).
        aggrisk = lognorm.sf(linerate * cutoff, zsd, 0, exp(zmu)) * 100
        if aggrisk > self.alarm_level:
            rpc_obj = {"jsonrpc": "2.0", "method": "overload_alarm", "params": {"overload_risk": aggrisk}}
            rpc_obj_json = json.dumps(rpc_obj)
            self.publish('monitor_alarm',rpc_obj_json)
            


    def perform_sql_DUMMY(self,sql):
        print('perform_sql: ' + str(sql))

    def perform_sql(self,sql):
        logging.info('perform_sql: ' + str(sql))
        try:
            logging.info('perform_sql: self.conn==' + repr(self.conn))
            curs = self.conn.cursor()
            curs.execute(sql)
        except psycopg2.Error as e:
            logging.info('perform_sql, execute error: ' + str(e))
            self.conn.rollback()
            raise e

    def perform_sql_commit(self):
        self.conn.commit()

    def perform_sql_with_commit(self,sql):
        self.perform_sql(sql)
        self.perform_sql_commit()

    def perform_sql_with_commit_error_as_warning(self,sql):
        try:
            self.perform_sql_with_commit(sql)
        except psycopg2.Error as e:
            logging.warning('sql error:' + str(e))


    # We should probably never have to use this method.
    def drop_all_functions(self):
        drop = dict()
        drop['sumfunc'] = "DROP FUNCTION sumfunc(float[],float[]) CASCADE;"
        drop['ffunc'] =  "DROP FUNCTION ffunc(float[]) CASCADE;"
        drop['dfunc'] = "DROP FUNCTION dfunc(float[]) CASCADE;"
        drop['pylogrisk'] = "DROP FUNCTION pylogrisk (float, float, integer, float) CASCADE;"
# ### [PD] Calculate the overload risk based on the aggregated lognormal
# ###      distribution in the calling python code instead. See the method
# ###      evaluate_overload_risk().
#        drop['ffuncrisk'] = "DROP FUNCTION ffuncrisk(float[]) CASCADE;"
        drop['lognormagg'] = "DROP AGGREGATE lognormagg(float[]) CASCADE;"
        drop['lognormrisk'] = "DROP AGGREGATE lognormrisk (float[]) CASCADE; "
        for key, sql_cmd in drop.items():
            try:
                self.perform_sql(sql_cmd)
            except psycopg2.Error as e:
                logging.warning('drop_all_functions, error: ' + str(e))
                # Keep going if the dropped thing does not exist.
        self.perform_sql_commit()

    def create_all_functions(self):
        create = OrderedDict()

# ### [PD] Since we calculate the overload risk based on the aggregated
# ###      lognormal distribution in the calling python code instead, we don't
# ###       need this extension. See the method evaluate_overload_risk().
#         create['1plpythonu'] = "CREATE extension plpythonu WITH SCHEMA pg_catalog;"
# 
#         create['2pylogrisk'] = """CREATE OR REPLACE FUNCTION pylogrisk (zmu float, zsd float, linerate integer, cutoff float)
#   RETURNS float
# AS $$
# from math import exp,pow,log,sqrt
# from scipy.stats import lognorm
# return lognorm.sf(linerate * cutoff, zsd, 0, exp(zmu)) * 100
# $$ LANGUAGE plpythonu;"""

        create['3sumfunc'] = """CREATE OR REPLACE FUNCTION sumfunc(acc float[], next float[]) RETURNS float[] AS
'select ARRAY[acc[1] + exp(2*next[1]+next[2]^2)*(exp(next[2]^2-1)),
        acc[2] + exp(next[1]+(next[2]^2)/2)];'
    LANGUAGE SQL
    IMMUTABLE
    RETURNS NULL ON NULL INPUT;"""

        create['4ffunc'] = """
CREATE OR REPLACE FUNCTION ffunc(float[]) RETURNS float[]
 AS 'select ARRAY[ln($1[2]) - ln($1[1] / ($1[2]^2)  +1)/2, sqrt(ln($1[1] / $1[2]^2   +1))];'
    LANGUAGE SQL
    IMMUTABLE
    RETURNS NULL ON NULL INPUT;"""

# ### [PD] Calculate the overload risk based on the aggregated lognormal 
# ###      distribution in the calling python code instead. See the method
# ###      evaluate_overload_risk().
#         create['5ffuncrisk'] = """CREATE OR REPLACE FUNCTION ffuncrisk(float[]) RETURNS float
#  AS 'select pylogrisk(ln($1[2]) - ln($1[1] / ($1[2]^2)  +1)/2, sqrt(ln($1[1] / $1[2]^2)   +1),100,0.95);'
#     LANGUAGE SQL
#     IMMUTABLE
#     RETURNS NULL ON NULL INPUT;"""

# There is no "CREATE OR REPLACE" for AGGREGATE
#        create['6lognormagg'] = """CREATE OR REPLACE AGGREGATE lognormagg (float[]) (
        create['6lognormagg'] = """CREATE AGGREGATE lognormagg (float[]) (
sfunc = sumfunc,
stype = float[],
finalfunc = ffunc,
initcond = '{0.0,0.0}'
);"""

# ### [PD] Calculate the overload risk based on the aggregated lognormal
# ###      distribution in the calling python code instead. See the method
# ###      evaluate_overload_risk().
# ###      This code is wrong, anyway. The risk should be based on the CDF of
# ###      the aggregated distribution.
# # There is no "CREATE OR REPLACE" for AGGREGATE
# #        create['7lognormrisk'] = """CREATE OR REPLACE AGGREGATE lognormrisk (float[]) (
#         create['7lognormrisk'] = """CREATE AGGREGATE lognormrisk (float[]) (
# sfunc = sumfunc,
# stype = float[],
# finalfunc = ffuncrisk,
# initcond = '{0.0,0.0}'
# );"""

#        create['8activate'] = "ACTIVATE;"
        activate = "ACTIVATE;"

        for key, sql_cmd in create.items():
            try:
                logging.info('create_all_functions: ' + key)
                self.perform_sql_with_commit(sql_cmd)
            except psycopg2.Error as e:
                logging.warning('sql error: ' + str(e))
                # Keep going if the extension or function already exists.
                # For the time being we don't handle other errors ...

        # ACTIVATE cannot run inside a transaction block.
        prev_autocommit = self.conn.autocommit
        self.conn.autocommit = True
        self.perform_sql(activate)
        self.conn.autocommit = prev_autocommit

    def setup_all_continuous_views(self):
        for name in ['m1','m2','m3','m4']:
            self.setup_all_continuous_views(name)

    def setup_continuous_view(self,name):
        create_cv = "CREATE CONTINUOUS VIEW %s AS SELECT count(*) FROM stream_%s"%(name,name)
        self.perform_sql_with_commit_error_as_warning(create_cv)

    def remove_continuous_view(self,name):
        drop_cv = "DROP CONTINUOUS VIEW %s"%name
        self.perform_sql_with_commit_error_as_warning(drop_cv)

    def create_stream(self,name):
        create_str = "CREATE STREAM stream_%s (lm float, lsd float)"%name
        self.perform_sql_with_commit_error_as_warning(create_str)

    def remove_stream(self,name):
        drop_str = "DROP STREAM stream_%s"%name
        self.perform_sql_with_commit_error_as_warning(drop_str)

    def insert_into_stream(self,name,lm,lsd):
        insert_string = 'INSERT INTO stream_%s'%name
        insert_string = insert_string + " (lm, lsd) VALUES (%s,%s)"%(lm,lsd)
        self.perform_sql_with_commit(insert_string)

    def on_reg(self):
        msg = dict()
        msg["type"] = "reg"
        logging.info(json.dumps(msg))
        for topic in self.mytopics:
            logging.info("topic: " + repr(topic))
            self.subscribe(*topic)

    def on_discon(self):
        msg = dict()
        msg["type"] = "discon"
        logging.info(json.dumps(msg))


    def on_error(self, code, error):
        msg = dict()
        msg["type"] = "error"
        msg["code"] = code.decode()
        msg["error"] = error.decode()
        logging.error(json.dumps(msg))

    def on_pub(self, src, topic, data):
        msg = dict()
        msg["type"] = "pub"
        msg["src"] = src.decode()
        msg["topic"] = topic.decode()
        msg["data"] = data.decode()
        logging.info(json.dumps(msg))
        rpc_dict = json.loads(data.decode())
        aggregate(rpc_dict)


    def aggregate(rpc_dict):

# Members of a Json-RPC object.
#
# jsonrpc:
# A String specifying the version of the JSON-RPC protocol. MUST be
# exactly "2.0".
#
        if ("2.0" not in rpc_dict['jsonrpc']):
            logging.error("Json-Rpc version error: Expecting 2.0, received " + str(rpc_dict['jsonrpc']))
            sys.exit(1)
            
# params:
# A Structured value that holds the parameter values to be used during
# the invocation of the method. This member MAY be omitted.
#
#        pdb.set_trace()
        try:
            params = rpc_dict['params']
        # In this case we can't to anything without params
        except KeyError as e:
            logging.error("Json-Rpc error: params missing in request object. Object is " + str(rpc_dict))
            sys.exit(1)

# method:
# A String containing the name of the method to be invoked. Method
# names that begin with the word rpc followed by a period character
# (U+002E or ASCII 46) are reserved for rpc-internal methods and
# extensions and MUST NOT be used for anything else.
#
        if (rpc_dict['method'] == 'rate_data'):
            self.rate_data(params)
        elif (rpc_dict['method'] == 'add_monitor'):
            self.add_monitor(params)
        elif (rpc_dict['method'] == 'remove_monitor'):
            self.remove_monitor(params)
        elif (rpc_dict['method'] == 'set_alarm_level'):
            self.set_alarm_level(params)
        else:
            logging.error('unknown method: ' + rpc_dict['method'])
        
# id:
# An identifier established by the Client that MUST contain a String,
# Number, or NULL value if included. If it is not included it is
# assumed to be a notification. The value SHOULD normally not be Null
# [1] and Numbers SHOULD NOT contain fractional parts [2]

#        pdb.set_trace()
        try:
            rpc_dict['id']
        # There should not be an 'id' member of the Json-Rpc call since the
        # call is a notification.
            logging.error("Json-Rpc warning: id present in request object, but should not be. Object is " + str(rpc_dict))

        except KeyError as e:
            pass                # Everything is hunky-dory
        return



    def on_stdin(self,fp,*kargs):
        data = fp.readline()
        try:
            res = json.loads(data)
        except ValueError as e:
            res = dict()

        if "type" in res:
            if res['type'] == "notify":
                self.sendmsg(res['dst'],res['data'])
            elif res['type'] == "pub":
                self.publish(res['topic'], res['data'])
            elif res['type'] == "sub":
                self.subscribe(res['topc'],res['scope'])
            else:
                print(json.dumps({"type":"error", "data":"Command '%s' not implemented"%res['type']}))
        else:
            print(json.dumps({"type":"error", "data":"Couldn't parse JSON"}))


    def run(self):
        self.start()


    def exit_program(self,button):
        self.shutdown()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Aggregator client")
    parser.add_argument('name', help="Identity of this client")
    parser.add_argument('customer', help="Name of the customer to get the keys (i.e. 'a' for the customer-a.json file)")
    parser.add_argument(
        '-d',
        "--dealer",
        help='URL to connect DEALER socket to, "tcp://1.2.3.4:5555"',
        nargs='?',
        default='tcp://127.0.0.1:5555')
    parser.add_argument(
        '-u',
        "--unsafe",
        action='store_true',
        help='aggregator client',
        default=False)
    parser.add_argument(
        '-f',
        "--logfile",
        help='File to write logs to',
        nargs='?',
        default=None)
    parser.add_argument(
        '-l',
        "--loglevel",
        help='Set loglevel (DEBUG, INFO, WARNING, ERROR, CRITICAL)',
        nargs='?',
        default="INFO")

    parser.add_argument(
        '-k',
        "--keyfile",
        help='File containing the encryption/authentication keys)',
        nargs='?',
        default='')

    parser.add_argument(
        '-t',
        "--topics",
        help='Comma separated list of topics/scopes, e.g. "topic1/all,topic2/node"',
        nargs='?',
        default='')

    parser.add_argument(
        '-b',
        "--dbhost",
        help='Name or IP# of the database host',
        nargs='?',
        default=None)

    parser.add_argument(
        '-p',
        "--dbport",
        help='Port number of the database',
        nargs='?',
        default=None)

    parser.add_argument(
        '-n',
        "--dbname",
        help='Name of the database',
        nargs='?',
        default='pipeline')

    parser.add_argument(
        '-e',
        "--dbuser",
        help='User name for the database',
        nargs='?',
        default='pipeline')

    parser.add_argument(
        '-w',
        "--dbpassword",
        help='Password for the database',
        nargs='?',
        default='pipeline')

    args = parser.parse_args()

    numeric_level = getattr(logging, args.loglevel.upper(), None)
    if not isinstance(numeric_level, int):
        raise ValueError('Invalid log level: %s' % args.loglevel)
    if args.logfile:
        logging.basicConfig(format='%(levelname)s:%(message)s', filename=args.logfile, level=numeric_level)
    else:
        logging.basicConfig(format='%(levelname)s:%(message)s', filename=args.logfile, level=numeric_level)

# These environmant variables are available when starting in a Docker
# linked to another Docker in which the PipelineDB is running.
# #$ env | grep PIPE
# PIPELINEDB_PORT_5432_TCP_PROTO=tcp
# PIPELINEDB_PORT_5432_TCP_ADDR=172.17.0.4
# PIPELINEDB_PORT=tcp://172.17.0.4:5432
# PIPELINEDB_PORT_5432_TCP_PORT=5432
# PIPELINEDB_NAME=/drunk_mcnulty/pipelinedb
# PIPELINEDB_ENV_DEBIAN_FRONTEND=noninteractive
# PIPELINEDB_PORT_5432_TCP=tcp://172.17.0.4:5432
# PIPELINEDB_ENV_LANG=en_US.utf8

#    pdb.set_trace()
    if args.dbhost is None:
        env_dbhost = os.getenv("PIPELINEDB_PORT_5432_TCP_ADDR",None)
        if env_dbhost is None:
            dbhost = "127.0.0.1"
        else:
            dbhost = env_dbhost
    else:
        dbhost = args.dbhost
    if args.dbport is None:
        env_dbport = os.getenv("PIPELINEDB_PORT_5432_TCP_PORT",None)
        if env_dbport is None:
            dbport = "5432"
        else:
            dbport = env_dbport
    else:
        dbport = args.dbport

    logging.info("Aggregator client")
    genclient = Aggregator(name=args.name, dealerurl=args.dealer, customer=args.customer,keyfile=args.keyfile, topics=args.topics, dbname = args.dbname, dbuser = args.dbuser, dbpass = args.dbpassword, dbhost = dbhost, dbport = dbport)

    logging.info("Starting DoubleDecker aggregator client")
    genclient._IOLoop.add_handler(sys.stdin,genclient.on_stdin,genclient._IOLoop.READ)

    genclient.run()
