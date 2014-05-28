#pragma once
#include "stdafx.h"
#include "network.h"
#include "../error/error.h"

namespace augs {
	namespace network {
		overlapped_accept::overlapped_accept(augs::threads::overlapped_userdata* new_userdata) : augs::threads::overlapped(new_userdata), accepted(INVALID_SOCKET) {
			memset(this, 0, sizeof(this));
		}

		overlapped_accept::~overlapped_accept() {
			if(accepted != INVALID_SOCKET) closesocket(accepted);
			accepted = INVALID_SOCKET;
		}

		bool overlapped_accept::create_acceptable() {
			if(accepted != INVALID_SOCKET) return true;
			return ((tcp*)(&accepted))->open();
		}

		tcp::tcp() : sock(INVALID_SOCKET) {}

		tcp::~tcp() { 
			close();
		}
		
		bool tcp::open() {
			close();
			return err((sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED)) != INVALID_SOCKET) != 0;
		}

		bool tcp::open(overlapped_accept& r) {
			close();
			sock = r.accepted;
			r.accepted = INVALID_SOCKET;

			sockaddr* remote, *local;
			int nremote, nlocal;

			tcp::getacceptexsockaddrs(
				&r.addr_local,
				0,
				sizeof(r.addr_local),
				sizeof(r.addr_remote),
				&local,
				&nlocal,
				&remote,
				&nremote);

			memcpy(&addr.addr, remote, nremote);
			return err((sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED)) != INVALID_SOCKET) != 0;
		}

		bool tcp::nagle(bool onoff) {
			return err(!setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&onoff, sizeof(bool))) != 0;
		}

		bool tcp::linger(bool onoff, int timeout) {
			LINGER l;
			l.l_linger = timeout;
			l.l_onoff = onoff;
			return err(!(setsockopt(sock, SOL_SOCKET, SO_LINGER, (char*)&l, sizeof(LINGER)))) != 0;
		}
		
		bool tcp::bind(unsigned short port, char* ipv4) {
			addr = ip(port,  ipv4);
			return err(::bind(sock, (SOCKADDR*) &addr.addr, sizeof (addr.addr)) != SOCKET_ERROR) != 0;
		}

		bool tcp::connect(ip& to, overlapped* request) {
			return err(connectex(sock, (sockaddr*)&to.addr, sizeof(to.addr), 0, 0, 0, &request->overlap) || WSAGetLastError() == WSA_IO_PENDING) != 0;
		}

		bool tcp::listen(int backlog) {
			return err(::listen(sock, backlog) != SOCKET_ERROR) != 0;
		}

		bool tcp::accept(overlapped_accept* request) {
			request->create_acceptable();
			return err(acceptex(sock, request->accepted, request->addr_local, 0, sizeof(request->addr_local),  sizeof(request->addr_remote), &request->result, &request->overlap) || WSAGetLastError() == WSA_IO_PENDING) != 0; 
		}

		int tcp::send(const wsabuf& b, overlapped* request) {
			return err((WSASend(sock, (LPWSABUF)&b, 1, &request->result, request->flags, &request->overlap, 0) == 0) ? 2 : WSAGetLastError() == WSA_IO_PENDING );
		}
		
		int tcp::send(const wsabuf* bufs, int bufcnt, overlapped* request) {
			return err((WSASend(sock, (LPWSABUF)bufs, bufcnt, &request->result, request->flags, &request->overlap, 0) == 0) ? 2 : WSAGetLastError() == WSA_IO_PENDING );
		}

		int tcp::recv(wsabuf& b, overlapped* request) {
			return err((WSARecv(sock, (LPWSABUF)&b, 1, &request->result, &request->flags, &request->overlap, 0) == 0) ? 2 : WSAGetLastError() == WSA_IO_PENDING ); 
		}

		int tcp::recv(wsabuf* bufs, int bufcnt, overlapped* request) {
			return err((WSARecv(sock, (LPWSABUF)bufs, bufcnt, &request->result, &request->flags, &request->overlap, 0) == 0) ? 2 : WSAGetLastError() == WSA_IO_PENDING ); 
		}
		

		bool tcp::send(const wsabuf& b, unsigned long& result, unsigned long flags) {
			return err(!(WSASend(sock, (LPWSABUF)&b, 1, &result, flags, 0, 0))) != 0;
		}
		
		bool tcp::send(const wsabuf* bufs, int bufcnt, unsigned long& result, unsigned long flags) {
			return err(!(WSASend(sock, (LPWSABUF)bufs, bufcnt, &result, flags, 0, 0))) != 0;
		}

		bool tcp::recv(wsabuf& b, unsigned long& result, unsigned long& flags) {
			return err(!(WSARecv(sock, (LPWSABUF)&b, 1, &result, &flags, 0, 0))) != 0; 
		}

		bool tcp::recv(wsabuf* bufs, int bufcnt, unsigned long& result, unsigned long& flags) {
			return err(!(WSARecv(sock, (LPWSABUF)bufs, bufcnt, &result, &flags, 0, 0))) != 0; 
		}
		

		bool tcp::close() {
			bool res = true;
			if(sock != INVALID_SOCKET) {
				res = !closesocket(sock);
				sock = INVALID_SOCKET;
			}
			return err(res) != 0;
		}

	}
}