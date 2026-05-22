for i in {1..999}; do
  (echo "test message" | nc localhost 8081 &)
done
