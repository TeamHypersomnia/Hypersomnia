#pragma once
#include "stdafx.h"
#include "udp.h"
#include "../error/error.h"

namespace augs {
	namespace network {

		udp::udp() : sock(INVALID_SOCKET) {}

		udp::~udp() {
			close();
		}

		bool udp::open() {
			auto res = err(((sock = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED)) != INVALID_SOCKET)) != 0;
			SetFileCompletionNotificationModes((HANDLE)sock, 0);
			return res;
		}

		bool udp::bind(unsigned short port = 0, char* ipv4) {
			addr = ip(port, ipv4);
			return err(::bind(sock, (struct sockaddr*)&addr.addr, sizeof(addr)) == 0) != 0;
		}

		void udp::set_blocking(bool flag) {
			DWORD nonBlocking = flag ? 0 : 1;
			err(ioctlsocket(sock,
				FIONBIO,
				&nonBlocking) == NO_ERROR);
		}

		int udp::send(const ip& to, const wsabuf& b, unsigned long& result) {
			unsigned long flags = 0;
			auto call_result = WSASendTo(sock, (LPWSABUF) &b, 1, &result, flags, (SOCKADDR*) &to.addr, to.size, 0, 0);
			err(call_result == 0);

			switch (call_result) {
			case 0: return io_result::SUCCESSFUL;
			case WSAEWOULDBLOCK: return io_result::INCOMPLETE;
			default: return io_result::FAILED;
			}
		}

		int udp::recv(ip& from, wsabuf& b, unsigned long& result) {
			unsigned long flags = 0;
			auto call_result = (WSARecvFrom(sock, (LPWSABUF) &b, 1, &result, &flags, (SOCKADDR*) &from.addr, &from.size, 0, 0));
			err(call_result == 0);

			switch (call_result) {
			case 0: return io_result::SUCCESSFUL;
			case WSAEWOULDBLOCK: return io_result::INCOMPLETE;
			default: return io_result::FAILED;
			}
		}

		udp::send_result::send_result() : result(-1), bytes_transferred(0ul) {}

		udp::send_result udp::send(const ip& to, packet& input_packet) {
			wsabuf input_buf;
			input_buf.set(input_packet.data.data(), input_packet.data.size());

			send_result res;
			res.result = send(to, input_buf, res.bytes_transferred);

			return std::move(res);
		}

		udp::recv_result udp::recv() {
			wsabuf input_buf;
			input_buf.set(buffer_for_receives, sizeof(buffer_for_receives));

			recv_result res;
			res.result = recv(res.sender_address, input_buf, res.bytes_transferred);

			res.message.data.assign((char*)input_buf.get(), (char*)(input_buf.get()) + input_buf.get_len());

			return std::move(res);
		}

		bool udp::get_result(overlapped& t) const {
			return err(WSAGetOverlappedResult(sock, &t.overlap, &t.result, false, &t.flags) || WSAGetLastError() == WSA_IO_INCOMPLETE) != 0;
		}

		bool udp::close() {
			bool res = true;
			if(sock != INVALID_SOCKET) {
				res = !closesocket(sock);
				sock = INVALID_SOCKET;
			}
			return err(res) != 0;
		}
	}
}
