#include "HttpDownloader.h"
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
	std::string url, des;

	if(argc < 2){
		std::cout << "No URL found.\n";
		return 0;
	}
	url = argv[1];

	if(argc >= 3)
		des = argv[2];
	HttpDownloader downloader(url, des, 20);
	downloader.getFile();
	return 0;
}
