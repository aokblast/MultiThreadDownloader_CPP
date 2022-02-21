//
// Created by aokblast on 2022/2/18.
//

#ifndef MULTITHREADDOWNLOADER_HTTPDOWNLOADER_H
#define MULTITHREADDOWNLOADER_HTTPDOWNLOADER_H

#include <string>
#include <thread>
#include <atomic>

class HttpDownloader {
public:
	HttpDownloader(const std::string &url, const std::string &des, const int threads = 10, const int timeOut = 1000);
	void getFile();


private:

	class DownloadingThread{

	public:
		DownloadingThread(int id, int start, int end, const HttpDownloader& downloader);

		void getPortionThread();


	private:
		bool download();

		const HttpDownloader& downloader;
		int id;
		int start;
		int end;
	};

	std::string url;
	std::string des;
	int threadNums;
	int timeOut;
	int fileSize;
	mutable std::atomic_int remainThreads = 0;
	mutable std::atomic_int downloadSize = 0;
	std::thread monitorThread;
	const static int BUFSIZE = 1025;

	bool isMultiThreadAndGetSize();
	void startMonitor();
	void clearAndMergeAllTmpFile();
};


#endif //MULTITHREADDOWNLOADER_HTTPDOWNLOADER_H
