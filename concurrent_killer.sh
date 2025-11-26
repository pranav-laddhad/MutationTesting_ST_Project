#!/bin/bash

# --- Concurrency Killer Script (concurrent_killer.sh) ---

compile_server() {
    echo "Compiling server $1..."
    # Now that the function is named 'main', use standard compilation:
    gcc server.c -o $1 -pthread 
    if [ $? -ne 0 ]; then
        echo "Server compilation failed."
        exit 1
    fi
}

# 1. Setup: Clean up old files and compile
make clean > /dev/null
compile_server mutant_server
gcc client.c -o client -pthread # Compile client
if [ $? -ne 0 ]; then
    echo "Client compilation failed."
    exit 1
fi
rm -f books.txt # Ensure file is clean before test

# 2. Launch the Mutant Server in the background
echo "--- 🚀 Launching MUTANT Server (Lock Deleted) ---"
./mutant_server &
SERVER_PID=$!
sleep 1 # Give server time to bind to port

# 3. Define the concurrent client actions
# Function to run the client and send Admin login + Add Book command
run_client() {
    CLIENT_ID=$1
    DELAY=$2
    BOOK_TITLE="BookTitle${CLIENT_ID}"
    BOOK_AUTHOR="Author${CLIENT_ID}"
    
    echo "--- Client $CLIENT_ID trying to add $BOOK_TITLE ---"
    
    # Input sequence: 2 (Admin) -> admin -> admin -> 1 (Add) -> Title -> Author -> 5 (Exit)
    (sleep $DELAY; echo 2; echo admin; echo admin; echo 1; echo $BOOK_TITLE; echo $BOOK_AUTHOR; echo 5) | ./client 
}

# 4. Execute the clients concurrently
echo "--- 🔥 Launching Clients Concurrently (Race Condition Test) ---"

# Launch first client immediately in background
run_client 1 0 & 
P1_PID=$!

# Launch second client almost immediately in background
run_client 2 0.05 &
P2_PID=$!

# 5. Wait for clients to finish
wait $P1_PID $P2_PID

# 6. Stop the server gracefully
echo "--- Stopping Server (PID: $SERVER_PID) ---"
kill $SERVER_PID
wait $SERVER_PID 2>/dev/null

# 7. Verification: Check the books.txt file
echo "--- 🔎 Verifying books.txt for Corruption ---"
if [ ! -f books.txt ]; then
    echo "books.txt not found. Server likely crashed or didn't create the file."
    exit 1
fi

BOOK_COUNT=$(wc -l < books.txt)
ID_1_COUNT=$(grep "^1 " books.txt | wc -l)
ID_2_COUNT=$(grep "^2 " books.txt | wc -l)

echo "Total records found: $BOOK_COUNT"
echo "Count of ID 1 records: $ID_1_COUNT"
echo "Count of ID 2 records: $ID_2_COUNT"

# The original logic (with lock) would produce 1 record of ID 1 and 1 record of ID 2 (or 2 and 3 if file existed).
# The expected failure for the mutant is two records with the same ID (e.g., both ID 1)

if [ "$BOOK_COUNT" -eq 2 ] && [ "$ID_1_COUNT" -eq 2 ]; then
    echo "--- ❌ MUTANT KILLED: Race Condition Detected! ---"
    echo "Two records found, but both were assigned the same ID (ID 1). The missing lock caused data corruption."
    exit 1
elif [ "$BOOK_COUNT" -eq 2 ] && [ "$ID_1_COUNT" -eq 1 ] && [ "$ID_2_COUNT" -eq 1 ]; then
    echo "--- ✅ MUTANT SURVIVED: Race Condition NOT detected ---"
    echo "The threads successfully serialized without the lock (lucky timing). Try rerunning the script."
    exit 0
else
    echo "--- ❓ UNEXPECTED RESULT ---"
    cat books.txt
    exit 0
fi