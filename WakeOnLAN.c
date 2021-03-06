/*
 * WakeOnLAN v0.3
 * A simple C program that sends a magic packet
 *
 *
 * MIT License
 * 
 * Copyright (c) 2017 Grammatopoulos Athanasios-Vasileios
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */


#ifdef __linux
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>
#endif
#ifdef _WIN32
	#include <windows.h>
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#pragma comment(lib, "Ws2_32.lib")
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

// Create Magic Packet
void createMagicPacket(unsigned char packet[], unsigned int macAddress[]){
	int i;
	// Mac Address Variable
	unsigned char mac[6];

	// 6 x 0xFF on start of packet
	for(i = 0; i < 6; i++){
		packet[i] = 0xFF;
		mac[i] = macAddress[i];
	}
	// Rest of the packet is MAC address of the pc
	for(i = 1; i <= 16; i++){
		memcpy(&packet[i * 6], &mac, 6 * sizeof(unsigned char));
	}
}

// Main Program
int main(int argc, const char* argv[]){
	// Default broadcast address
	char broadcastAddress[16] = "192.168.1.255";
	// Packet buffer
	unsigned char packet[102];
	// Mac address
	unsigned int mac[6];
	// Set broadcast
	int broadcast = 1;

	// Linux socket
	#ifdef __linux
		// Socket
		int udpSocket;
		// Socket address
		struct sockaddr_in udpClient;
		struct sockaddr_in udpServer;
	#endif

	// Windows socket
	#ifdef _WIN32
		// Socket data
		WSADATA data;
		// Socket
		SOCKET udpSocket;
		// Socket address
		struct sockaddr_in udpClient;
		struct sockaddr_in udpServer;
	#endif

	// Help variables
	int i = 0;
	
	// If no arguments
	if(argc < 2){
		printf("Usage:\n./wakeonlan <mac address> (<broadcast address>)\n");
		exit(EXIT_FAILURE);
	}

	// Parse Mac Address
	i = sscanf(argv[1],"%x:%x:%x:%x:%x:%x", &(mac[0]), &(mac[1]), &(mac[2]), &(mac[3]), &(mac[4]), &(mac[5]));
	if(i != 6){
		printf("Invalid mac address was given.\n");
		exit(EXIT_FAILURE);
	}

	// Print address
	printf("Packet will be sent to %02hhx:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	// Check if a broadcast address was given too
	if(argc > 2){
		// Parse Broadcast Address
		i = sscanf(argv[2],"%d.%d.%d.%d", &i, &i, &i, &i);
		if(i == 4){
			strcpy(broadcastAddress, argv[2]);
		}
	}
	printf("Broadcast address %s will be used.\n", broadcastAddress);

	// Create Magic Packet
	createMagicPacket(packet, mac);

	// Linux socket
	#ifdef __linux
		// Setup broadcast socket
		udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
		if(setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1){
			printf("Failed to setup a broadcast socket.\n");
			exit(EXIT_FAILURE);
		}
		// Set parameters
		udpClient.sin_family = AF_INET;
		udpClient.sin_addr.s_addr = INADDR_ANY;
		udpClient.sin_port = 0;
		// Bind socket
		bind(udpSocket, (struct sockaddr*) &udpClient, sizeof(udpClient));

		// Set server end point (the broadcast addres)
		udpServer.sin_family = AF_INET;
		udpServer.sin_addr.s_addr = inet_addr(broadcastAddress);
		udpServer.sin_port = htons(9);

		// Send the packet
		sendto(udpSocket, &packet, sizeof(unsigned char) * 102, 0, (struct sockaddr*) &udpServer, sizeof(udpServer));
	#endif

	// Windows socket
	#ifdef _WIN32
		// Setup broadcast socket
		WSAStartup(MAKEWORD(2, 2), &data);
		udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, (char *) &broadcast, sizeof(broadcast)) == -1){
			printf("Failed to setup a broadcast socket.\n");
			exit(EXIT_FAILURE);
		}
		// Set parameters
		udpClient.sin_family = AF_INET;
		udpClient.sin_addr.s_addr = INADDR_ANY;
		udpClient.sin_port = htons(0);
		// Bind socket
		bind(udpSocket, (struct sockaddr*) &udpClient, sizeof(udpClient));

		// Set server end point (the broadcast addres)
		udpServer.sin_family = AF_INET;
		udpServer.sin_addr.s_addr = inet_addr(broadcastAddress);
		udpServer.sin_port = htons(9);

		// Send the packet
		sendto(udpSocket, packet, sizeof(unsigned char) * 102, 0, (struct sockaddr*) &udpServer, sizeof(udpServer));
	#endif

	// Done
	printf("Wake up packet was sent.\n");
	exit(EXIT_SUCCESS);
}
