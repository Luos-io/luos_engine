from simple_websocket_server import WebSocketServer, WebSocket
import json
import argparse

PING = 0
END = 1
OK = 2
NOK = 3

class WsBroker(WebSocket):
    pinged = False
    next_node = None
    global pinger

    def handle(self):
        if len(self.data) == 1:
            if len(pinger) == 0:
                # Data should be a ping
                if self.data[0] != PING:
                    print("Error: received data is not a ping, received data is " + str(self.data[0]))
                else:
                    pinger.append(self)
                    find_someone = False
                    # This is a ping command, find the next unpinged node and ping it
                    for client in clients:
                        if client != self and client.pinged == False:
                            # We have someone to ping
                            find_someone = True
                            client.pinged = True
                            client.send_message([PING])
                            # ack the ping to the sender
                            self.send_message([OK])
                            break
                    if find_someone == False:
                        # We have no one to ping, this branch is finished, we can send a NOK to this ping and reset the pinged state of all nodes
                        self.send_message([NOK])
                        pinger.remove(pinger[0])
                        for client in clients:
                            client.pinged = False
            else:
                # Data should be an end
                if self.data[0] != END:
                    print("Error: received data is not an end, received data is " + str(self.data[0]) + " from " + str(self.address))
                else:
                    # send the end to the pinger
                    pinger[0].send_message([END])
                    #remove the pinger
                    pinger.remove(pinger[0])
        else:
            # This is a broadcast message
            #print(str(len(self.data)) + str(" Data received from " + str(self.address)))
            for client in clients:
                if client != self:
                    client.send_message(self.data)

    def connected(self):
        print(self.address, 'connected\n')
        clients.append(self)

    def handle_close(self):
        print(self.address, 'closed')
        clients.remove(self)


## Parse arguments ##
parser = argparse.ArgumentParser(description='Luos_engine WebSocket network broker\n',
                                 formatter_class=argparse.RawTextHelpFormatter)
# General arguments
parser.add_argument("-p", "--port", metavar="PORT", action="store",
                    help="The port used by the websocket.\n"
                    "By default port = 8000.\n",
                    default=8000)
parser.add_argument("--ip", metavar="IP", action="store",
                    help="The ip used by the websocket.\n"
                    "By default ip = '127.0.0.1'.\n",
                    default='127.0.0.1')

args = parser.parse_args()
clients = []
pinger = []

server = WebSocketServer(args.ip, args.port, WsBroker)
print("Luos_engine WebSocket network broker opened on " +
      str(args.ip) + ":" + str(args.port))
server.serve_forever()
