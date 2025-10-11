CXX = g++
CXXFLAGS = -std=c++17 -Iinclude
LDFLAGS = -lcurl

SRC = src/main.cpp src/contracts.cpp src/http_utils.cpp
OUT = contract_fetcher

$(OUT): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)