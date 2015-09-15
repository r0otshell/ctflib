import sys
import socket

BIND_HOST = '0.0.0.0'
BIND_PORT = 9007

COIN_WEIGHT = 10

def split_coins(coins):

    coins_left_start = 0
    coins_left_end =  len(coins) / 2
    coins_right_start = coins_left_end
    coins_right_end = len(coins)

    coins_left = coins[coins_left_start:coins_left_end]
    coins_right = coins[coins_right_start:coins_right_end]

    return coins_left, coins_right

def get_expected_weight(coins):

    return COIN_WEIGHT * len(coins)

def get_actual_weight(coins):

    coins = coin_str(coins)
    data = send_coins(coins)
    return int(data.strip())

def send_coins(coins):

    con.send('%s\n' % coins)
    data = con.recv(32)
    return data

def coin_str(coins):
    return ' '.join(coins)

def search(coins):

    coins_left, coins_right = split_coins(coins)

    coins_left_expected_weight = get_expected_weight(coins_left)
    coins_left_actual_weight = get_actual_weight(coins_left)

    if coins_left_expected_weight != coins_left_actual_weight:
        if len(coins_left) == 1:
            return coins_left[0]
        return search(coins_left)
    else:
        if len(coins_right) == 1:
            return coins_right[0]
        return search(coins_right)

def recv_intro():

    while True:

        line = con.recv(4096)
        return line

def recv_start_data():

    data = con.recv(32)
    if 'time expired' in data or 'ngrats' in data:
        return data

    dlist = data.split()

    num_coins = int(dlist[0][2:])

    return num_coins

def connect_to_server():

    con = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    con.connect((BIND_HOST, BIND_PORT))

    return con

def create_coin_list(num_coins):

    return [ '%d' % c for c in xrange(0, num_coins) ]

def send_result(result):

    con.send('%s\n' % result)
    data = con.recv(256)
    return data

def get_flag(first_part):

    data = con.recv(4096)
    print '[coinsauce]: GOT EEEEM!'
    print '[coinsauce]: Fuck yeahh, computer science!'
    print '[server]: %s%s' % (first_part, data)

if __name__ == '__main__':

    con = connect_to_server()

    print recv_intro()

    while True:

        num_coins = recv_start_data()
        if type(num_coins) == type(''):

            if 'ngrats' in num_coins:
                get_flag(num_coins)
            else:
                print '[coinsauce]: Time expired... quitting'

            sys.exit(0)

        coins = create_coin_list(num_coins)

        result = search(coins)
        data = send_result(result)
        while 'Correct' not in data:
            data = send_result(result)
        print '[server]:', data

