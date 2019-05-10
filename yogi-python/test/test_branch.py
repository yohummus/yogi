# This file is part of the Yogi distribution https://github.com/yohummus/yogi.
# Copyright (c) 2018 Johannes Bergmann.
#
# This library is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this library. If not, see <http://www.gnu.org/licenses/>.

import yogi
import unittest
import socket
import os
import uuid

from .common import TestCase


class TestBranches(TestCase):
    def setUp(self):
        self.context = yogi.Context()
        self.json_view = yogi.JsonView("[1,2,3]")
        self.big_json_view = yogi.JsonView(self.make_big_json_data())
        self.msgpack_view = yogi.MsgpackView(bytes([0x93, 0x1, 0x2, 0x3]))

    def make_big_json_data(self, size=10000):
        return '["' + '.' * (size - 5) + '"]'

    def test_branch_events_enum(self):
        for ev in yogi.BranchEvents:
            if ev is yogi.BranchEvents.ALL:
                self.assertFlagCombinationMatches("YOGI_BEV_", ev, [])
            else:
                self.assertFlagMatches("YOGI_BEV_", ev)

    def test_create_with_sub_section(self):
        branch = yogi.Branch(
            self.context, '{"branch":{"name":"Cow"}}', "/branch")
        self.assertEqual(branch.name, "Cow")

    def test_info(self):
        props = {
            "name": "My Branch",
            "description": "Stuff",
            "network_name": "My Network",
            "network_password": "Password",
            "path": "/some/path",
            "advertising_address": "239.255.0.1",
            "advertising_port": 12345,
            "advertising_interval": 7,
            "timeout": -1
        }

        branch = yogi.Branch(self.context, props)

        info = branch.info
        self.assertIsInstance(info, yogi.LocalBranchInfo)
        self.assertIsInstance(info.uuid, uuid.UUID)
        self.assertEqual(info.name, "My Branch")
        self.assertEqual(info.description, "Stuff")
        self.assertEqual(info.network_name, "My Network")
        self.assertEqual(info.path, "/some/path")
        self.assertEqual(info.hostname, socket.gethostname())
        self.assertEqual(info.pid, os.getpid())
        self.assertEqual(info.advertising_address, "239.255.0.1")
        self.assertEqual(info.advertising_port, 12345)
        self.assertEqual(info.advertising_interval, 7.0)
        self.assertGreater(len(info.tcp_server_address), 1)
        self.assertGreater(info.tcp_server_port, 0)
        self.assertLessEqual(info.start_time, yogi.get_current_time())
        self.assertEqual(info.timeout, float("inf"))
        self.assertEqual(info.ghost_mode, False)
        self.assertGreater(info.tx_queue_size, 1000)
        self.assertGreater(info.rx_queue_size, 1000)

        for key in info._info:
            if key not in ["advertising_interfaces"]:
                self.assertEqual(getattr(branch, key), info._info[key])

    def test_get_connected_branches(self):
        branch = yogi.Branch(self.context, '{"name":"My Branch"}')
        branch_a = yogi.Branch(self.context, '{"name":"A"}')
        branch_b = yogi.Branch(self.context, '{"name":"B"}')

        while len(branch.get_connected_branches()) < 2:
            self.context.run_one()
        branches = branch.get_connected_branches()

        for brn in [branch_a, branch_b]:
            self.assertTrue(brn.uuid in branches)
            self.assertEqual(branches[brn.uuid].name, brn.name)
            self.assertIsInstance(branches[brn.uuid], yogi.RemoteBranchInfo)

    def test_await_event(self):
        branch = yogi.Branch(self.context, '{"name":"My Branch"}')
        branch_a = yogi.Branch(self.context, '{"name":"A"}')

        fn_res = None
        fn_event = None
        fn_evres = None
        fn_info = None
        fn_called = False

        def fn(res, event, evres, info):
            nonlocal fn_res, fn_event, fn_evres, fn_info, fn_called
            fn_res = res
            fn_event = event
            fn_evres = evres
            fn_info = info
            fn_called = True

        branch.await_event_async(yogi.BranchEvents.BRANCH_QUERIED |
                                 yogi.BranchEvents.CONNECTION_LOST, fn)

        while not fn_called:
            self.context.run_one()

        self.assertIsInstance(fn_res, yogi.Success)
        self.assertEqual(fn_res.error_code, yogi.ErrorCode.OK)
        self.assertIsInstance(fn_event, yogi.BranchEvents)
        self.assertEqual(fn_event, yogi.BranchEvents.BRANCH_QUERIED)
        self.assertIsInstance(fn_evres, yogi.Success)
        self.assertEqual(fn_evres.error_code, yogi.ErrorCode.OK)
        self.assertIsInstance(fn_info, yogi.BranchQueriedEventInfo)
        self.assertEqual(fn_info.uuid, branch_a.uuid)
        self.assertEqual(fn_info.start_time, branch_a.start_time)
        self.assertEqual(fn_info.timeout, branch_a.timeout)
        self.assertEqual(fn_info.ghost_mode, branch_a.ghost_mode)

    def test_cancel_await_event(self):
        branch = yogi.Branch(self.context, '{"name":"My Branch"}')

        self.assertFalse(branch.cancel_await_event())

        fn_res = None
        fn_event = None
        fn_evres = None
        fn_info = None
        fn_called = False

        def fn(res, event, evres, info):
            nonlocal fn_res, fn_event, fn_evres, fn_info, fn_called
            fn_res = res
            fn_event = event
            fn_evres = evres
            fn_info = info
            fn_called = True

        branch.await_event_async(yogi.BranchEvents.ALL, fn)
        self.assertTrue(branch.cancel_await_event())

        self.context.poll()

        self.assertIsInstance(fn_res, yogi.Failure)
        self.assertEqual(fn_res.error_code, yogi.ErrorCode.CANCELED)
        self.assertIsInstance(fn_event, yogi.BranchEvents)
        self.assertEqual(fn_event, yogi.BranchEvents.NONE)
        self.assertIsInstance(fn_evres, yogi.Success)
        self.assertEqual(fn_evres.error_code, yogi.ErrorCode.OK)
        self.assertEqual(fn_info, None)

    def test_send_broadcast(self):
        branch_a = yogi.Branch(
            self.context, '{"name": "a", "_transceive_byte_limit": 5}')
        branch_b = yogi.Branch(self.context, '{"name": "b"}')
        self.run_context_until_branches_are_connected(
            self.context, [branch_a, branch_b])
        self.context.run_in_background()

        # Receive a broadcast to verify that it has actually been sent
        broadcast_received = False

        def rcv_handler(res, source, payload):
            self.assertEqual(res.error_code, yogi.ErrorCode.OK)
            self.assertEqual(payload, self.big_json_view)
            nonlocal broadcast_received
            broadcast_received = True

        branch_b.receive_broadcast_async(rcv_handler,
                                         encoding=yogi.EncodingType.JSON)

        # Blocking
        for _ in range(3):
            self.assertTrue(branch_a.send_broadcast(
                self.big_json_view, block=True))

        # Non-blocking:
        while branch_a.send_broadcast(self.big_json_view, block=False):
            pass

        self.context.stop()
        self.context.wait_for_stopped()

        # Verify that a broadcast has actually been sent
        while not broadcast_received:
            self.context.run_one()

    def test_send_broadcast_async(self):
        branch_a = yogi.Branch(
            self.context, '{"name": "a", "_transceive_byte_limit": 5}')
        branch_b = yogi.Branch(self.context, '{"name": "b"}')
        self.run_context_until_branches_are_connected(
            self.context, [branch_a, branch_b])

        # Receive a broadcast to verify that it has actually been sent
        broadcast_received = False

        def rcv_handler(res, source, payload):
            self.assertEqual(res.error_code, yogi.ErrorCode.OK)
            self.assertEqual(payload, self.big_json_view)
            nonlocal broadcast_received
            broadcast_received = True

        branch_b.receive_broadcast_async(rcv_handler,
                                         encoding=yogi.EncodingType.JSON)

        # Send with retry = True
        results = []

        def snd_retry_handler(res, oid):
            self.assertEqual(res.error_code, yogi.ErrorCode.OK)
            self.assertTrue(oid.is_valid)
            results.append(res)

        n = 3
        for _ in range(n):
            oid = branch_a.send_broadcast_async(self.big_json_view,
                                                snd_retry_handler, retry=True)
            self.assertTrue(oid.is_valid)

        while len(results) != n:
            self.context.poll()

        # Send with retry = false
        def snd_no_retry_handler(res, _):
            results.append(res)

        while results[-1].error_code == yogi.ErrorCode.OK:
            branch_a.send_broadcast_async(self.big_json_view,
                                          snd_no_retry_handler, retry=False)
            self.context.poll_one()

        self.assertEqual(results[-1].error_code, yogi.ErrorCode.TX_QUEUE_FULL)

        # Verify that a broadcast has actually been sent
        while not broadcast_received:
            self.context.run_one()

    def test_cancel_send_broadcast(self):
        branch_a = yogi.Branch(
            self.context, '{"name": "a", "_transceive_byte_limit": 5}')
        branch_b = yogi.Branch(self.context, '{"name": "b"}')
        self.run_context_until_branches_are_connected(
            self.context, [branch_a, branch_b])

        oid_to_ec = {}

        def handler(res, oid):
            oid_to_ec[oid] = res.error_code

        while True:
            oid = branch_a.send_broadcast_async(self.big_json_view, handler)
            self.assertTrue(oid.is_valid)

            oid_to_ec[oid] = yogi.ErrorCode.UNKNOWN

            if branch_a.cancel_send_broadcast(oid):
                self.context.poll()
                self.assertEqual(oid_to_ec[oid], yogi.ErrorCode.CANCELED)
                break

    def test_receive_broadcast(self):
        branch_a = yogi.Branch(self.context, '{"name": "a"}')
        branch_b = yogi.Branch(self.context, '{"name": "b"}')
        self.run_context_until_branches_are_connected(
            self.context, [branch_a, branch_b])

        # Simplest form
        uuid_b = branch_b.uuid
        called = False

        def handler_1(res, source, payload):
            self.assertEqual(res.error_code, yogi.ErrorCode.OK)
            self.assertEqual(source, uuid_b)
            self.assertEqual(payload, self.msgpack_view)
            nonlocal called
            called = True

        branch_a.receive_broadcast_async(handler_1)

        branch_b.send_broadcast_async(self.msgpack_view, lambda x, y: None)
        while not called:
            self.context.run_one()

        # With buffer in handler function
        called = False

        def handler_2(res, source, payload, buffer):
            self.assertEqual(res.error_code, yogi.ErrorCode.OK)
            self.assertEqual(source, uuid_b)
            self.assertEqual(payload, self.msgpack_view)
            self.assertGreaterEqual(len(buffer),
                                    yogi.Constants.MAX_MESSAGE_PAYLOAD_SIZE)
            nonlocal called
            called = True

        branch_a.receive_broadcast_async(handler_2)

        branch_b.send_broadcast_async(self.msgpack_view, lambda x, y: None)
        while not called:
            self.context.run_one()

        # With encoding
        called = False

        def handler_3(res, _, payload):
            self.assertEqual(res.error_code, yogi.ErrorCode.OK)
            self.assertEqual(payload, self.json_view)
            nonlocal called
            called = True

        branch_a.receive_broadcast_async(handler_3,
                                         encoding=yogi.EncodingType.JSON)

        branch_b.send_broadcast_async(self.msgpack_view, lambda x, y: None)
        while not called:
            self.context.run_one()

        # With custom buffer
        buffer = bytearray(123)
        called = False

        def handler_4(res, _, payload, buf):
            self.assertEqual(res.error_code, yogi.ErrorCode.OK)
            self.assertEqual(payload, self.msgpack_view)
            self.assertIs(buf, buffer)
            nonlocal called
            called = True

        branch_a.receive_broadcast_async(handler_4, buffer=buffer)

        branch_b.send_broadcast_async(self.msgpack_view, lambda x, y: None)
        while not called:
            self.context.run_one()

    def test_cancel_receive_broadcast(self):
        branch = yogi.Branch(self.context, '{"name": "a"}')

        self.assertFalse(branch.cancel_receive_broadcast())

        called = False

        def handler(res, source, payload):
            self.assertEqual(res.error_code, yogi.ErrorCode.CANCELED)
            nonlocal called
            called = True

        branch.receive_broadcast_async(handler)

        self.assertTrue(branch.cancel_receive_broadcast())
        self.context.poll()
        self.assertTrue(called)


if __name__ == '__main__':
    unittest.main()
