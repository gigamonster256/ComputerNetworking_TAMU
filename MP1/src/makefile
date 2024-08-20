all: echos echo
clean :
	rm -f echos
	rm -f echo

echos: server.cpp echo.hpp
	$(CXX) $(CXXFLAGS) -o echos server.cpp

echo: client.cpp echo.hpp
	$(CXX) $(CXXFLAGS) -o echo client.cpp


