# Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
# Systems
"""
TODO : Sample string decribing the purpose of this file.
"""

import time

import spdlog as spd
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol

from buzzblog.gen import TAccountService


def instrumented(func):
    """ TODO : Method description """
    def func_wrapper(self, request_metadata, *args, **kwargs):
        start_time = time.monotonic()
        ret = func(self, request_metadata, *args, **kwargs)
        latency = time.monotonic() - start_time
        try:
            logger = spd.get("logger")
            logger.info(f'request_id={request_metadata.id} \
                      server={self._ip_address}:{self._port} \
                      function=account:{func.__name__} latency={latency:.9f}')
        except NotImplementedError:
            pass
        return ret
    return func_wrapper

class Client:
    """ TODO : Class description """
    def __init__(self, ip_address, port, timeout=10000):
        self._ip_address = ip_address
        self._port = port
        self._socket = TSocket.TSocket(ip_address, port)
        self._socket.setTimeout(timeout)
        self._transport = TTransport.TBufferedTransport(self._socket)
        self._protocol = TBinaryProtocol.TBinaryProtocol(self._transport)
        self._tclient = TAccountService.Client(self._protocol)
        self._transport.open()

    def __enter__(self):
        return self

    def __exit__(self, exception_type, exception_value, exception_traceback):
        self.close()

    def __del__(self):
        """ TODO : Method description """
        self.close()

    def close(self):
        """ TODO : Method description """
        if self._transport.isOpen():
            self._transport.close()

    @instrumented
    def authenticate_user(self, request_metadata, username, password):
        """ TODO : Method description """
        return self._tclient.authenticate_user(request_metadata=request_metadata,
            username=username, password=password)

    @instrumented
    def create_account(self, request_metadata, username, password, first_name,
        last_name):
        """ TODO : Method description """
        return self._tclient.create_account(request_metadata=request_metadata,
            username=username, password=password, first_name=first_name,
            last_name=last_name)

    @instrumented
    def retrieve_standard_account(self, request_metadata, account_id):
        """ TODO : Method description """
        return self._tclient.retrieve_standard_account(
            request_metadata=request_metadata, account_id=account_id)

    @instrumented
    def retrieve_expanded_account(self, request_metadata, account_id):
        """ TODO : Method description """
        return self._tclient.retrieve_expanded_account(
            request_metadata=request_metadata, account_id=account_id)

    @instrumented
    def update_account(self, request_metadata, account_id, password, first_name,
        last_name):
        """ TODO : Method description """
        return self._tclient.update_account(request_metadata=request_metadata,
            account_id=account_id, password=password, first_name=first_name,
            last_name=last_name)

    @instrumented
    def delete_account(self, request_metadata, account_id):
        """ TODO : Method description """
        return self._tclient.delete_account(request_metadata=request_metadata,
            account_id=account_id)
