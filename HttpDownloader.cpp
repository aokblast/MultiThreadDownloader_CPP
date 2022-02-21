//
// Created by aokblast on 2022/2/18.
//

#include "HttpDownloader.h"
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Infos.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>
#include <filesystem>

template<typename ... Args>
static std::string string_format(const std::string& format, Args ... args){
	size_t size = 1 + snprintf(nullptr, 0, format.c_str(), args ...);  // Extra space for \0
	// unique_ptr<char[]> buf(new char[size]);
	char bytes[size];
	snprintf(bytes, size, format.c_str(), args ...);
	return std::string(bytes);
}


HttpDownloader::HttpDownloader(const std::string &_url, const std::string &_des, const int _theadNums, const int _timeOut) {
	this->des = _des == "" ? _url.substr(_url.rfind('/') + 1) : des;
	this->url = _url;
	this->threadNums = _theadNums;
	this->timeOut = _timeOut;

}

void HttpDownloader::getFile() {

	auto timeUsed = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

	cURLpp::initialize();

	std::vector<std::thread *> threads;

	if(!isMultiThreadAndGetSize()){
		threads.push_back(new std::thread(
				&DownloadingThread::getPortionThread,
				DownloadingThread(0, 0, fileSize - 1, *this)));
	}else{
		int blockSize = fileSize / threadNums;
		int startPoints[threadNums + 1];

		for(int i = 0; i < threadNums; ++i){
			startPoints[i] = blockSize * i;
		}
		startPoints[threadNums] = fileSize;

		for(int i = 0; i < threadNums; ++i){
			threads.push_back(new std::thread(
					&DownloadingThread::getPortionThread,
					DownloadingThread(i, startPoints[i], startPoints[i + 1] - 1, *this)
				));
			++remainThreads;
		}
	}

	startMonitor();

	monitorThread.join();

	clearAndMergeAllTmpFile();

	timeUsed = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) - timeUsed;

	std::cout << "File download successfully.\n";

	std::cout << "Time used: " << std::setprecision(3) << timeUsed <<
	", Average download speed: " << downloadSize / timeUsed / 1000.0 << "KB/s\n";


	cURLpp::terminate();
}

bool HttpDownloader::isMultiThreadAndGetSize() {
	cURLpp::Easy handler;
	std::stringstream ss;

	handler.setOpt(cURLpp::Options::Url(url));
	handler.setOpt(cURLpp::Options::Range("0-"));
	handler.setOpt(cURLpp::Options::WriteStream(&ss));
	handler.setOpt(cURLpp::Options::Header(true));
	handler.setOpt(cURLpp::Options::NoBody(true));

	long resCode = 0;

	while(true){
		try {
			handler.perform();
			resCode = cURLpp::infos::ResponseCode::get(handler);
			std::string header = ss.str();
			fileSize = std::stoi(header.substr(header.find("Content-Length") + 15));
			break;
		}catch (cURLpp::RuntimeError &e){
			std::cout << e.what() << '\n';
		}
	}

	return resCode == 206;
}


void HttpDownloader::startMonitor() {

	using namespace std::chrono_literals;

	monitorThread = std::thread([&]{
		int preDownloadSize = 0, curDownloadSize = 0;
		while(true){
			std::this_thread::sleep_for(1000ms);
			curDownloadSize = downloadSize;
			int curThreads = remainThreads;
			std::cout << string_format("Speed: %d KB/s, Current Download Size: %d KB(%lf%%), Remaining Thread(s): %d\n",
									   (curDownloadSize - preDownloadSize) >> 10, curDownloadSize >> 10, 1.0 * curDownloadSize / fileSize * 100,
									   curThreads);
			if(remainThreads == 0)
				break;
			preDownloadSize = curDownloadSize;
		}
	});
}


void HttpDownloader::clearAndMergeAllTmpFile() {
	std::ofstream output(des, std::ios::binary);

	if(threadNums == 1){
		std::filesystem::rename(string_format("%s.%d.tmp", des.c_str(), 0), des);
	}else{
		for(int i = 0; i < threadNums; ++i){
			std::string curFileName = string_format("%s.%d.tmp", des.c_str(), i);
			std::ifstream input(curFileName, std::ios::binary);

			std::copy(
					std::istreambuf_iterator<char>(input),
					std::istreambuf_iterator<char>( ),
					std::ostreambuf_iterator<char>(output));

			std::filesystem::remove(curFileName);
		}
	}
	output.close();
}

HttpDownloader::DownloadingThread::DownloadingThread(int _id, int _start, int _end, const HttpDownloader& _downloader) : downloader(_downloader){
	this->id = _id;
	this->start = _start;
	this->end = _end;
}

void HttpDownloader::DownloadingThread::getPortionThread() {
	bool success = false;

	while(!success){
		success = download();
		if(!success){
			std::cout << "Download part " << id << " unsuccessful, trying re-download.\n";
		}
	}

	std::cout << "Download part " << id << " successful.\n";
}

bool HttpDownloader::DownloadingThread::download() {
	cURLpp::Easy connection;
	std::stringstream ss;
	try{
		connection.setOpt(cURLpp::Options::Url(downloader.url));
		connection.setOpt(cURLpp::Options::Range(string_format("%d-%d", start, end)));
		connection.setOpt(cURLpp::Options::Timeout(downloader.timeOut));
		connection.setOpt(cURLpp::Options::WriteStream(&ss));

		connection.perform();

		// TODO: Check if Content-Len == segmentSize
		/*
		std::string result = ss.str();

		int getSize = cURLpp::infos::ContentLengthDownload::get(connection);
		std::cout << getSize << '\n';
		if(getSize != end - start + 1)
			return false;
		*/
	}catch (cURLpp::RuntimeError &e){
		std::cout << e.what() << '\n';
		return false;
	}

	std::ofstream output(string_format("%s.%d.tmp", downloader.des.c_str(), id), std::ios::binary);
	downloader.downloadSize += end - start + 1;
	output << ss.str();
	output.close();
	--downloader.remainThreads;
	return true;

}

