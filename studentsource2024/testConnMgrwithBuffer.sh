make connWithBuff
port=5678
clients=3
echo -e "Starting gateway..."
valgrind --leak-check=full ./connBuff.out $port $clients &  # Start connection manager
sleep 3

echo -e 'Starting 3 sensor nodes - from shell'
./sensor_node 1 1 127.0.0.1 $port &  # Start sensor node 1
sleep 1
./sensor_node 2 3 127.0.0.1 $port &  # Start sensor node 2
sleep 2
# Incorrect port should not connect
./sensor_node 100 2 127.0.0.1 1234 &  # Incorrect port for sensor node
sleep 2
./sensor_node 3 2 127.0.0.1 $port &  # Start sensor node 3
sleep 2
# Excess nodes
./sensor_node 4 2 127.0.0.1 $port &  # Start sensor node 4 (should be ignored by connection manager)
sleep 2
./sensor_node 5 2 127.0.0.1 $port &  # Start sensor node 5
sleep 2
./sensor_node 6 2 127.0.0.1 $port &  # Start sensor node 6
sleep 2

# Terminate sensor nodes
killall sensor_node
sleep 5

# Terminate connection manager
killall connBuff.out
