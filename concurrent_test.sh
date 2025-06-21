LOG_FILE="concurrent_test.log"
CLIENTS_COUNT=$1

START_TIME=$(gdate +%s.%N)

echo "테스트 시작\n" > $LOG_FILE
echo "실행할 클라이언트 수: $CLIENTS_COUNT" >> $LOG_FILE
echo "----------------------------------------" >> $LOG_FILE

# 여러 클라이언트 실행
for ((i=1; i<=CLIENTS_COUNT; i++)); do
    ./one-card.app/Contents/MacOS/one-card > /dev/null 2>&1 & # 백그라운드로 실행
    PID=$!
    echo "클라이언트 $i 실행 (PID: $PID)" >> $LOG_FILE
    
    # PID 배열에 저장
    PIDS[$i]=$PID
done

END_TIME=$(gdate +%s.%N)
DURATION=$(echo "$END_TIME - $START_TIME" | bc)
DURATION=$(printf "%.4f" $DURATION)

echo "----------------------------------------" >> $LOG_FILE
echo "모든 클라이언트 실행 완료" >> $LOG_FILE
echo "총 실행 시간: ${DURATION}초" >> $LOG_FILE

cat $LOG_FILE

{ for pid in ${PIDS[*]}; do
    kill -9 $pid 
done } > /dev/null 2>&1

echo "테스트 완료"