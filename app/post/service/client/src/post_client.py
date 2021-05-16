# Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
# Systems

import time

import spdlog as spd
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol

from buzzblog.gen import TPostService


def instrumented(func):
  def func_wrapper(self, request_metadata, *args, **kwargs):
    logger = spd.get("logger")
    start_time = time.monotonic()
    ret = func(self, request_metadata, *args, **kwargs)
    latency = time.monotonic() - start_time
    logger.info("request_id=%s server=%s:%s function=post:%s latency=%.9f" %
        (request_metadata.id, self._ip_address, self._port, func.__name__,
            latency))
    return ret
  return func_wrapper


class Client:
  def __init__(self, ip_address, port, timeout=10000):
    self._ip_address = ip_address
    self._port = port
    self._socket = TSocket.TSocket(ip_address, port)
    self._socket.setTimeout(timeout)
    self._transport = TTransport.TBufferedTransport(self._socket)
    self._protocol = TBinaryProtocol.TBinaryProtocol(self._transport)
    self._tclient = TPostService.Client(self._protocol)
    self._transport.open()

  def __enter__(self):
    return self

  def __exit__(self, exception_type, exception_value, exception_traceback):
    self.close()

  def __del__(self):
    self.close()

  def close(self):
    if self._transport.isOpen():
      self._transport.close()

  @instrumented
  def create_post(self, request_metadata, text):
    return self._tclient.create_post(request_metadata=request_metadata,
        text=text)

  @instrumented
  def retrieve_standard_post(self, request_metadata, post_id):
    return self._tclient.retrieve_standard_post(
        request_metadata=request_metadata, post_id=post_id)

  @instrumented
  def retrieve_expanded_post(self, request_metadata, post_id):
    return self._tclient.retrieve_expanded_post(
        request_metadata=request_metadata, post_id=post_id)

  @instrumented
  def delete_post(self, request_metadata, post_id):
    return self._tclient.delete_post(request_metadata=request_metadata,
        post_id=post_id)

  @instrumented
  def list_posts(self, request_metadata, query, limit, offset):
    return self._tclient.list_posts(request_metadata=request_metadata,
        query=query, limit=limit, offset=offset)
