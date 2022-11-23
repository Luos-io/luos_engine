from simple_websocket_server import WebSocketServer, WebSocket
import json


class RobusEmulator(WebSocket):
    prev_node = None
    ptpa = False  # Linked to prev
    ptpb = False  # Linked to next
    ptpa_poke = False
    ptpb_poke = False
    next_node = None
    msg_index = 0

    def handle(self):
        if isinstance(self.data, str):
            # This is a PTP command
            print("\nI receive : " + str(self.data) +
                  " from " + str(self.address))

            # PTP emulation:
            # Because PTP have been designed for real time response, the Robus algorythm is not really friendly to PTP management over WebSocket.
            # This broker have to drive data in a specific way allowing to make it work.
            # Robus HAL will send messages only during PTP reset state and read line.
            # Read_line mean Poke. When we have this we can set the line depending on the presence of another node and save this poke state on the line
            # The next reset received will need to be send to the other node.
            #
            # if (ptp line read (PTP up) :
            #     if (a node is connected) :
            #         send state 1 to the other node
            #         send state 1 back
            #         pass this ptp to poking
            #     else :
            #         send state 0 back
            # if (PTP down and ptp is poking) :
            #     send state to the other node

            # PTPA
            if self.data[3] == 'A':
                # We get a PTPA data

                if (self.data[4] == '1'):
                    if (self.prev_node != None):
                        print("\t\tPTPB1 val sent to the node",
                              str(self.prev_node.address))
                        self.prev_node.send_message("PTPB1")
                        print("\t\tPTPA1 val sent back to the node",
                              str(self.address))
                        self.send_message("PTPA1")
                        self.prev_node.ptpb_poke = True
                        self.ptpa_poke = True
                    else:
                        print("\t\tPTPA0 val sent back to the node",
                              str(self.address))
                        self.send_message("PTPA0")

                if (self.data[4] == '0' and self.ptpa_poke == True and self.prev_node != None):
                    print("\t\tPTPB0 val sent to the node",
                          str(self.prev_node.address))
                    self.prev_node.send_message("PTPB0")
                    self.prev_node.ptpb_poke = False
                    self.ptpa_poke = False

            # PTPB
            if self.data[3] == 'B':
                # We get a PTPB data

                if (self.data[4] == '1'):
                    if (self.next_node != None):
                        print("\t\tPTPA1 val sent to the node",
                              str(self.next_node.address))
                        self.next_node.send_message("PTPA1")
                        print("\t\tPTPB1 val sent back to the node",
                              str(self.address))
                        self.send_message("PTPB1")
                        self.next_node.ptpa_poke = True
                        self.ptpb_poke = True
                    else:
                        print("\t\tPTPB0 val sent back to the node",
                              str(self.address))
                        self.send_message("PTPB0")

                if (self.data[4] == '0' and self.ptpb_poke == True and self.next_node != None):
                    print("\t\tPTPA0 val sent to the node",
                          str(self.next_node.address))
                    self.next_node.send_message("PTPA0")
                    self.next_node.ptpa_poke = False
                    self.ptpb_poke = False

        else:
            # This is a broadcast message
            print(str(self.msg_index)+" Data received from " + str(self.address))
            self.msg_index += 1
            for client in clients:
                if client != self:
                    client.send_message(self.data)

    def connected(self):
        print(self.address, 'connected')
        clients.append(self)
        # Save links to other nodes
        if len(clients) >= 2:
            self.prev_node = clients[-2]
            self.prev_node.next_node = clients[-1]
            print("connect PTPB of " + str(self.prev_node.address) +
                  " with PTPA of " + str(self.address))

    def handle_close(self):
        print(self.address, 'closed')
        # Save links to other nodes
        if self.next_node != None:
            self.next_node.prev_node = self.prev_node
        if self.prev_node != None:
            self.prev_node.next_node = self.next_node
        clients.remove(self)


clients = []

server = WebSocketServer('127.0.0.1', 8000, RobusEmulator)
server.serve_forever()
