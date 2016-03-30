#!/usr/bin/python3
# -*- coding: utf-8 -*-
import argparse
import logging
import json
import sys
import os
from collections import OrderedDict

from doubledecker.clientSafe import ClientSafe


# For PipelineDB integration
import psycopg2
import copy
from jsonrpcserver import dispatch, Methods
from jsonrpcserver.request import Request

Request.notification_errors = True

from pprint import pprint

# This program aggregates data from several monitoring functions
#
# Startup.
#
# If started from a command line you can set startup parameters via
# options. Do 'aggregator.py --help' for a list of startup parameters.
#
# If the environment variables PIPELINEDB_PORT_5432_TCP_ADDR and/or
# PIPELINEDB_PORT_5432_TCP_PORT are set, they are used if the
# corresponding startup options are not set.


class Aggregatorv2(ClientSafe):
    def __init__(self, name:str, dealerurl:str, customer:str, keyfile:str,
                 topics=['measurement/node'],
                 dbname="pipeline",
                 dbuser="pipeline",
                 dbpass="pipeline",
                 dbhost="127.0.0.1",
                 dbport="5432"):
        super(Aggregatorv2, self).__init__(name,dealerurl,customer,keyfile)
        self.mytopics = list()
        self.selects = dict()
        self.zstate = dict()
        self.zeval = dict()
        self.actions = dict()

        # Initialize the RPC-Server dispatcher
        self.methods = Methods()
        self.methods.add_method(self.rate_data)
        self.methods.add_method(self.add_monitor)
        self.methods.add_method(self.remove_monitor)
        self.methods.add_method(self.configure)
        self.methods.add_method(self.measurement)
        self.methods.add_method(self.ping)

        self.known_sources = list()

        try:
            for top in topics:
                if len(top) == 0:
                    return
                (topic, scope) = top.split("/")
                self.mytopics.append((topic, scope))
        except ValueError:
            logging.error("Could not parse topics!")
            sys.exit(1)
        logging.getLogger('jsonrpcserver').setLevel(logging.WARNING)
        connstr = 'dbname=' + dbname + ' user= ' + dbuser + ' password=' + dbpass + ' host=' + dbhost + ' port=' + dbport
        logging.info("Aggregator: attempting connection to " + connstr)
        self.conn = psycopg2.connect(connstr)
        self.pipeline = self.conn.cursor()
        #        self.drop_all_functions()
        self.create_all_functions()
        self.rates = Rates({})
        self.drop_all_cvs()

    def ping(self, ddsrc:str) -> str:
        return "OK"



    def handle_jsonrpc(self, src, msg, topic=None):
        request = json.loads(msg.decode('UTF-8'))


        if 'error' in request:
            logging.error(str(request['error']))
            return

        if 'result' in request:
            logging.info(str(request['result']))
            return

        # include the 'ddsrc' parameter so the
        # dispatched method knows where the message came from
        if 'params' not in request:
            request['params'] = {}

        request['params']['ddsrc'] = src.decode()
        response = dispatch(self.methods, request)

        # if the http_status is 200, its request/response, otherwise notification
        if response.http_status == 200:
            logging.info("Replying to %s with %s" % (str(src), str(response)))
            self.sendmsg(src, str(response))
        # notification, correctly formatted
        elif response.http_status == 204:
            pass
        # if 400, some kind of error
        # return a message to the sender, even if it was a notification
        elif response.http_status == 400:
            self.sendmsg(src, str(response))
            logging.error("Recived bad JSON-RPC from %s, error %s" % (str(src), str(response)))
        else:
            logging.error(
                "Recived bad JSON-RPC from %s \nRequest: %s\nResponose: %s" % (str(src), msg.decode(), str(response)))

    def notify(self, target, message):
        self.sendmsg(dst=target, msg=message)

    def configure(self, ddsrc, tools, zonedeps, actions, zonecmds):
        print("configure called with, ", ddsrc)
        print("tools ", tools)
        print("zonedeps ", zonedeps)
        print("actions ", actions)
        print("zonecmds ", zonecmds)

        for z in zonecmds:
            for s in zonecmds[z]['streams']:
                print("Create stream: ", s)
                try:
                    self.perform_sql_with_commit_silent(s)
                except psycopg2.ProgrammingError as e:
                    print("CREATE STREAM Caught exception ", e)

            print("Creating views: ", zonecmds[z]['create'])
            try:
                self.perform_sql_with_commit(zonecmds[z]['create'])
            except psycopg2.ProgrammingError as e:
                print("CREATE CONTIUOUS VIEW Caught exception ", e)

            for m in zonedeps[z]:
                if m in self.selects:
                    self.selects[m].append((zonecmds[z]['select'], z))
                else:
                    self.selects[m] = [(zonecmds[z]['select'], z)]
            pprint(["Selects: ", self.selects])

        for n in actions:
            if 'from' in n['state']:
                #           if n['state']['from'] in self.actions:
                #              self.actions[n['state']['from']].append((n['state'], n['calls']))
                #         else:
                #            self.actions[n['state']['from']] = [(n['state'], n['calls'])]

                if n['state']['to'] in self.actions:
                    self.actions[n['state']['to']].append((n['state'], n['calls']))
                else:
                    self.actions[n['state']['to']] = [(n['state'], n['calls'])]
            if 'in' in n['state']:
                if n['state']['in'] in self.actions:
                    self.actions[n['state']['in']].append((n['state'], n['calls']))
                else:
                    self.actions[n['state']['in']] = [(n['state'], n['calls'])]
            if 'enter' in n['state']:
                if n['state']['enter'] in self.actions:
                    self.actions[n['state']['enter']].append((n['state'], n['calls']))
                else:
                    self.actions[n['state']['enter']] = [(n['state'], n['calls'])]
            if 'leave' in n['state']:
                if n['state']['leave'] in self.actions:
                    self.actions[n['state']['leave']].append((n['state'], n['calls']))
                else:
                    self.actions[n['state']['leave']] = [(n['state'], n['calls'])]

        pprint(["ACTIONS:", self.actions])

        return "OK"

    def check_result_json(self, ddsrc, thresh):
        select_string = 'SELECT  "rate.rx" FROM view_%s WHERE "rate.rx" > %f;' % (ddsrc, thresh)
        return self.perform_sql_silent_select(select_string)

    def run_actions(self, cmds):
        for n in cmds:
            if n == 'Publish':
                # TODO: check that the **cmd[n] is valid
                self.publish(**cmds[n])
            elif n == 'Notify':
                self.notify(**cmds[n])
            else:
                logging.error("Unknown command in MEASURE: ", n)

    def check_results(self, ddsrc):
        newzstate = copy.deepcopy(self.zstate)

        for (select_str, zone) in self.selects[ddsrc]:
            newzstate[zone] = self.perform_sql_silent_select(select_str)
        #            print("Zone %s is "%zone,newzstate[zone])

        # print("old state: ", self.zstate, " new state: ", newzstate)
        for (select_str, zone) in self.selects[ddsrc]:
            # pprint(["############ actions: \n", self.actions[zone]])
            for (state, cmds) in self.actions[zone]:
                if 'from' in state:
                    if state['from'] in self.zstate and state['to'] in newzstate:
                        new_from = newzstate[state['from']]
                        old_from = self.zstate[state['from']]
                        new_to = newzstate[state['to']]
                        old_to = self.zstate[state['to']]

                        if new_to and not old_to and old_from and not new_from:
                            self.run_actions(cmds)
                            # print("Transition from %s to %s, should take actions: "%(state['from'],state['to']),cmds)

                if 'in' in state:
                    if state['in'] in newzstate:
                        if newzstate[state['in']] == True:
                            self.run_actions(cmds)
                        #                            print("IN  %s, should take actions: "%state['in'],cmds)
                if 'enter' in state:
                    if state['enter'] in newzstate and state['enter'] in self.zstate:
                        old = self.zstate[state['enter']]
                        new = newzstate[state['enter']]
                        if not old and new:
                            self.run_actions(cmds)

                        #                            print("Entered state %s, shoult take actions: "%state['enter'], cmds)

                if 'leave' in state:
                    if state['leave'] in newzstate and state['leave'] in self.zstate:
                        old = self.zstate[state['leave']]
                        new = newzstate[state['leave']]
                        if old and not new:
                            self.run_actions(cmds)

                        #                            print("Left state %s, shoult take actions: "%state['leave'], cmds)

        self.zstate = newzstate

    ## called when new monitoring data arrives
    ## keep track of configured streams in a nicer way ..
    def measurement(self, ddsrc, result):
        if ddsrc not in self.known_sources:
            logging.warning("############################")
            logging.warning("creating stream_%s" % ddsrc)
            logging.warning("############################")
            self.create_stream_json(ddsrc)
            self.known_sources.append(ddsrc)

        self.insert_into_stream_json(ddsrc, result)

        self.check_results(ddsrc)
        # if self.check_result_json(ddsrc,0.6):
        #    logging.warning("%s is breaking the threshold %f"%(ddsrc,0.5))

    def rate_data(self, params):
        lm = params['lm']
        lsd = params['lsd']
        name = params['name']
        self.insert_into_stream(name, lm, lsd)
        self.rates.set_monitor_complete(name)
        if self.rates.is_complete():
            self.evaluate_overload_risk()
            self.rates.reset()

    def add_monitor(self, params):
        name = params["name"]
        self.create_stream(name)
        self.setup_continuous_view(name)
        self.rates.add_monitor(name)

    def remove_monitor(self, params):
        self.remove_continuous_view(params["name"])

    # FIXME: implement this method!
    def evaluate_overload_risk(self):
        print('evaluate_overload_risk called!')

    def perform_sql_DUMMY(self, sql):
        print('perform_sql: ' + str(sql))

    def perform_sql(self, sql):
        logging.debug('perform_sql: ' + str(sql))
        try:
            logging.debug('perform_sql: self.conn==' + repr(self.conn))
            curs = self.conn.cursor()
            curs.execute(sql)
            # While testing ...
            try:
                rows = curs.fetchall()
                for row in rows:
                    logging.debug('perform_sql, result: ' + str(row))
            except psycopg2.Error as e:
                logging.debug('perform_sql, result error: ' + str(e))
                # end while testing

        except psycopg2.Error as e:
            logging.error('perform_sql, execute error: ' + str(e))
            self.conn.rollback()
            raise e

    def perform_sql_silent(self, sql):
        try:
            curs = self.conn.cursor()
            curs.execute(sql)
            # While testing ...
            try:
                rows = curs.fetchall()
            except psycopg2.Error as e:
                logging.warning('perform_sql, result error: ' + str(e))
                # end while testing

        except psycopg2.Error as e:
            logging.error('perform_sql, execute error: ' + str(e))
            self.conn.rollback()
            raise e

    def perform_sql_insert_silent(self, sql):
        try:
            curs = self.conn.cursor()
            curs.execute(sql)

        except psycopg2.Error as e:
            logging.error('perform_sql, execute error: ' + str(e))
            self.conn.rollback()
            raise e

    def perform_sql_silent_select(self, sql):
        logging.debug("perform_sql_silent_select %s" % sql)
        try:
            curs = self.conn.cursor()
            curs.execute(sql)
            # While testing ...
            try:
                i = 0
                for n in curs:
                    logging.debug("silent_select: got result %s" % (n))
                    i += 1
                if i > 0:
                    return True
                logging.debug("silent_select: no result")
                return False

            except psycopg2.Error as e:
                logging.warning('perform_silent_selct, result error: ' + str(e))
                # end while testing

        except psycopg2.Error as e:
            logging.error('perform_silent_select, execute error: ' + str(e))
            self.conn.rollback()
            raise e
        return False

    def drop_all_cvs(self):
        logging.debug("removing existing continuous views")
        sql = 'SELECT name FROM pipeline_query'
        views = list()
        try:
            logging.info('perform_sql: self.conn==' + repr(self.conn))
            curs = self.conn.cursor()
            curs.execute(sql)
            # While testing ...
            try:
                rows = curs.fetchall()
                for row in rows:
                    logging.debug('perform_sql, result: ' + row[0])
                    views.append(row[0])
            except psycopg2.Error as e:
                logging.warning('perform_sql, result error: ' + str(e))
                # end while testing

        except psycopg2.Error as e:
            logging.info('perform_sql, execute error: ' + str(e))
            self.conn.rollback()
            raise e
        for n in views:
            print("dropping view : ", n)
            sql = 'DROP CONTINUOUS VIEW %s' % n
            self.perform_sql_with_commit(sql)

    def perform_sql_commit(self):
        self.conn.commit()

    def perform_sql_with_commit(self, sql):
        self.perform_sql(sql)
        self.perform_sql_commit()

    def perform_sql_with_commit_silent(self, sql):
        self.perform_sql_silent(sql)
        self.perform_sql_commit()

    def perform_sql_insert_commit(self, sql):
        self.perform_sql_insert_silent(sql)
        self.perform_sql_commit()

    def perform_sql_with_commit_error_as_warning(self, sql):
        try:
            self.perform_sql_with_commit(sql)
        except psycopg2.Error as e:
            logging.warning('sql error:' + str(e))

    # We should probably never have to use this method.
    def drop_all_functions(self):
        drop = dict()
        drop['sumfunc'] = "DROP FUNCTION sumfunc(float[],float[]) CASCADE;"
        drop['ffunc'] = "DROP FUNCTION ffunc(float[]) CASCADE;"
        drop['dfunc'] = "DROP FUNCTION dfunc(float[]) CASCADE;"
        drop['pylogrisk'] = "DROP FUNCTION pylogrisk (float, float, integer, float) CASCADE;"
        drop['ffuncrisk'] = "DROP FUNCTION ffuncrisk(float[]) CASCADE;"
        drop['lognoragg'] = "DROP AGGREGATE lognoragg(float[]) CASCADE;"
        drop['lognormris'] = "DROP AGGREGATE lognormrisk (float[]) CASCADE; "
        for key, sql_cmd in drop.items():
            try:
                self.perform_sql(sql_cmd)
            except psycopg2.Error as e:
                logging.warning('drop_all_functions, error: ' + str(e))
                # Keep going if the dropped thing does not exist.
        self.perform_sql_commit()

    def create_all_functions(self):
        create = OrderedDict()

        create['1plpythonu'] = "CREATE extension plpythonu WITH SCHEMA pg_catalog;"

        create['2pylogrisk'] = """CREATE OR REPLACE FUNCTION pylogrisk (zmu float, zsd float, linerate integer, cutoff float)
  RETURNS float
AS $$
from math import exp,pow,log,sqrt
from scipy.stats import lognorm
return lognorm.sf(linerate * cutoff, zsd, 0, exp(zmu)) * 100
$$ LANGUAGE plpythonu;"""

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

        create['5ffuncrisk'] = """CREATE OR REPLACE FUNCTION ffuncrisk(float[]) RETURNS float
 AS 'select pylogrisk(ln($1[2]) - ln($1[1] / ($1[2]^2)  +1)/2, sqrt(ln($1[1] / $1[2]^2)   +1),100,0.95);'
    LANGUAGE SQL
    IMMUTABLE
    RETURNS NULL ON NULL INPUT;"""

        # There is no "CREATE OR REPLACE" for AGGREGATE
        #        create['6lognormagg'] = """CREATE OR REPLACE AGGREGATE lognormagg (float[]) (
        create['6lognormagg'] = """CREATE AGGREGATE lognormagg (float[]) (
sfunc = sumfunc,
stype = float[],
finalfunc = ffunc,
initcond = '{0.0,0.0}'
);"""

        # There is no "CREATE OR REPLACE" for AGGREGATE
        #        create['7lognormrisk'] = """CREATE OR REPLACE AGGREGATE lognormrisk (float[]) (
        create['7lognormrisk'] = """CREATE AGGREGATE lognormrisk (float[]) (
sfunc = sumfunc,
stype = float[],
finalfunc = ffuncrisk,
initcond = '{0.0,0.0}'
);"""

        #        create['8activate'] = "ACTIVATE;"
        activate = "ACTIVATE;"

        for key, sql_cmd in create.items():
            try:
                self.perform_sql_with_commit_silent(sql_cmd)
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
        for name in ['m1', 'm2', 'm3', 'm4']:
            self.setup_all_continuous_views(name)

    def setup_continuous_view(self, name):
        create_cv = "CREATE CONTINUOUS VIEW %s AS SELECT count(*) FROM stream_%s" % (name, name)
        self.perform_sql_with_commit_error_as_warning(create_cv)

    def remove_continuous_view(self, name):
        drop_cv = "DROP CONTINUOUS VIEW %s" % name
        self.perform_sql_with_commit_error_as_warning(drop_cv)

    def create_stream(self, name):
        create_str = "CREATE STREAM stream_%s (lm float, lsd float)" % name
        self.perform_sql_with_commit_error_as_warning(create_str)

    def remove_stream(self, name):
        drop_str = "DROP STREAM stream_%s" % name
        self.perform_sql_with_commit_error_as_warning(drop_str)

    def insert_into_stream(self, name, lm, lsd):
        insert_string = 'INSERT INTO stream_%s' % name
        insert_string = insert_string + " (lm, lsd) VALUES (%s,%s)" % (lm, lsd)
        self.perform_sql_with_commit(insert_string)

    def create_stream_json(self, name):
        create_str = "CREATE STREAM stream_%s (data json)" % name
        self.perform_sql_with_commit_error_as_warning(create_str)

    def create_view_json(self, name):
        create_view = "CREATE CONTINUOUS VIEW view_%s AS SELECT " \
                      "avg(CAST(data->>'overload.risk.rx' as float)) as \"overload.risk.rx\" ," \
                      "avg(CAST(data->>'overload.risk.tx' as float)) as \"overload.risk.tx\", " \
                      "avg(CAST(data->>'rate.rx' as float)) as \"rate.rx\", " \
                      "avg(CAST(data->>'rate.tx' as float)) as \"rate.tx\" " \
                      "from stream_%s;" % (name, name)

        self.perform_sql_with_commit_error_as_warning(create_view)

    # inserts results into a stream as JSON
    # no need to know what is actually in the results
    def insert_into_stream_json(self, ddsrc, result):
        insert_string = "INSERT INTO stream_%s (data) VALUES ('%s')" % (ddsrc, json.dumps(result['results']))
        self.perform_sql_insert_commit(insert_string)

    ## DoubleDecker callbacks
    def on_reg(self):

        logging.info("Registered to Broker!")
        #        for topic in self.mytopics:
        #           logging.info("topic: " + repr(topic))
        #          self.subscribe(*topic)

        logging.info("Subscribing to 'measurements' on 'node'")
        self.subscribe(topic="measurement", scope="node")
        logging.info("Subscribing to 'aggregator' on 'node'")
        self.subscribe(topic="aggregator", scope="node")

    #        self.create_stream_json('mmpt')
    #       self.create_view_json('mmpt')


    def on_discon(self):
        logging.warning("Disconnected from Broker!")

    def on_error(self, code: int, error: int):
        logging.error("Got error: %d %s" % (code, error))

    def on_pub(self, src, topic:str, msg:str):
        self.handle_jsonrpc(src=src, msg=msg, topic=topic)

    def on_data(self, src:str, msg:str):
        self.handle_jsonrpc(src, msg, topic=None)

    def run(self):
        self.start()

    def exit_program(self, button):
        self.shutdown()


# Keeps track of rate monitoring data sent by the monitors
class Rates():
    # This init method will probably never be used.
    # add_monitor() is more flexible.
    def __init__(self, monitors={"m1", "m2", "m3", "m4"}):
        self.monitors = {}
        for m in monitors:
            self.monitors[m] = False
        logging.debug('Rates is initialized')

    # Returns True if all monitors have sent their rate data
    def is_complete(self):
        result = False
        for m in self.monitors:
            result = self.monitors[m] or result
        logging.debug('Rates.is_complete() returning ' + str(result))
        logging.debug('Rates.monitors==%s' % self.monitors)
        return result

    # Mark a monitor as having sent its rate data
    def set_monitor_complete(self, name):
        self.monitors[name] = True
        logging.debug('Rates.set_monitor_complete(%s)' % name)
        logging.debug('Rates.monitors[%s]==%s' % (name, self.monitors[name]))

    # Set all monitors to incomplete
    def reset(self):
        for m in self.monitors:
            self.monitors[m] = False
        logging.debug('Rates.reset()')

    def add_monitor(self, name):
        self.monitors[name] = False
        logging.debug('Rates.add_monitor(%s)' % name)

    def remove_monitor(self, name):
        self.monitors.pop(name, None)
        logging.debug('Rates.remove_monitor(%s)' % name)


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
        env_dbhost = os.getenv("PIPELINEDB_PORT_5432_TCP_ADDR", None)
        if env_dbhost is None:
            dbhost = "127.0.0.1"
        else:
            dbhost = env_dbhost
    else:
        dbhost = args.dbhost
    if args.dbport is None:
        env_dbport = os.getenv("PIPELINEDB_PORT_5432_TCP_PORT", None)
        if env_dbport is None:
            dbport = "5432"
        else:
            dbport = env_dbport
    else:
        dbport = args.dbport

    logging.info("Aggregatorv2 client")
    genclient = Aggregatorv2(name=args.name, dealerurl=args.dealer, customer=args.customer, keyfile=args.keyfile,
                             topics=args.topics, dbname=args.dbname, dbuser=args.dbuser, dbpass=args.dbpassword,
                             dbhost=dbhost, dbport=dbport)

    logging.info("Starting DoubleDecker aggregator client")
    # genclient._IOLoop.add_handler(sys.stdin,genclient.on_stdin,genclient._IOLoop.READ)

    genclient.run()
