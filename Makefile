CXX = g++
CXXFLAGS = -std=c++17 -Iinclude -Werror -pthread
LDFLAGS = -lcurl

SRC = src/main.cpp src/contracts.cpp src/http_utils.cpp
OUT = build/contract_fetcher

$(OUT): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)