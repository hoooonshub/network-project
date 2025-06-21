CXX        := g++
CXXFLAGS  := -std=c++11 -Wall -I..

# 서버 소스 디렉터리
SRCDIR     := .
BINDIR     := .

# 소스 파일 목록
SRCS       := Card.cpp Deck.cpp GameSession.cpp Player.cpp ServerManager.cpp server.cpp
OBJS       := $(SRCS:.cpp=.o)

# 최종 실행 파일 이름
TARGET     := one-card-server

# 빌드 규칙
all: $(TARGET)

# .o 파일을 찍는 규칙
%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 링크 규칙 (pthread 필요)
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $(BINDIR)/$(TARGET) -lpthread

# 청소 (빌드 결과물 삭제)
clean:
	rm -f $(OBJS) $(TARGET)