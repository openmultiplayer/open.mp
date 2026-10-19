/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <zlib.h>
#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <functional>
#include <ghc/filesystem.hpp>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <utility>
#include <vector>

// Packs the registered models into compressed segments that a launcher can download before
// the player joins. A segment is a zlib stream of its files back to back, written to a temp
// folder and served from there. Files go into segments in the order they were registered, so
// models added later only change the last segment or two.
//
// With a lot of models compressing takes a while, so it's done on a background thread. Until
// it has finished the routes answer 503 and launchers use the normal downloads.
class ModelPack
{
public:
	enum class Status
	{
		None,
		Building,
		Ready,
	};

	struct Segment
	{
		std::string path;
		uint64_t size;
	};

	ModelPack(std::string modelsPath, uint16_t port)
		: modelsPath_(std::move(modelsPath))
		, dir_(emptyDirectory(modelsPath_, port))
		, worker_(&ModelPack::run, this)
	{
	}

	~ModelPack()
	{
		{
			std::lock_guard<std::mutex> lock(mutex_);
			stop_ = true;
		}
		wake_.notify_all();
		worker_.join();
		std::error_code ignored;
		ghc::filesystem::remove_all(dir_, ignored);
	}

	void add(const std::string& diskName, uint32_t crc, uint64_t size)
	{
		auto dot = diskName.rfind('.');
		if (dot == std::string::npos)
		{
			return;
		}
		std::string extension = diskName.substr(dot);
		std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c)
			{
				return char(std::tolower(c));
			});
		if (extension != ".dff" && extension != ".txd")
		{
			return;
		}

		char hex[16];
		snprintf(hex, sizeof hex, "0x%X", crc);
		std::string cacheName = hex + extension;

		std::lock_guard<std::mutex> lock(mutex_);
		if (!names_.insert(cacheName).second)
		{
			return;
		}
		entries_.push_back({ diskName, cacheName, crc, size });
		total_ += size;
		dirty_ = true;
		failed_ = false;
		wake_.notify_all();
	}

	bool acquireDownload()
	{
		if (downloads_.fetch_add(1) >= MaxDownloads)
		{
			downloads_.fetch_sub(1);
			return false;
		}
		return true;
	}

	void releaseDownload()
	{
		downloads_.fetch_sub(1);
	}

	Status index(std::string& out)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (built_)
		{
			out = built_->index;
		}
		return status();
	}

	Status segment(size_t number, Segment& out)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (built_ && number >= built_->segments.size())
		{
			return Status::None;
		}
		if (built_)
		{
			out = built_->segments[number];
		}
		return status();
	}

private:
	struct Entry
	{
		std::string diskName;
		std::string cacheName; // name in the SA-MP model cache: 0x<CRC32>.<ext>
		uint32_t crc;
		uint64_t size;
		size_t segment = 0;
	};

	struct Built
	{
		std::string index;
		std::vector<Segment> segments;
	};

	// The segment size is a power of two, big enough for about SegmentCount segments but never
	// over MaxSegmentBytes. It only changes when the models double in size, so adding a few
	// doesn't move the boundaries of the existing segments.
	static constexpr uint64_t SegmentCount = 20;
	static constexpr uint64_t MinSegmentBytes = uint64_t(1) << 20;
	static constexpr uint64_t MaxSegmentBytes = uint64_t(256) << 20;
	// Segments are big, don't let them use up every thread of the web server.
	static constexpr int MaxDownloads = 8;

	std::mutex mutex_;
	std::condition_variable wake_;
	std::string modelsPath_;
	ghc::filesystem::path dir_;
	std::vector<Entry> entries_;
	std::set<std::string> names_;
	uint64_t total_ = 0;
	bool dirty_ = false;
	bool failed_ = false;
	std::atomic<bool> stop_ { false };
	std::shared_ptr<const Built> built_;
	unsigned builds_ = 0;
	std::atomic<int> downloads_ { 0 };
	std::thread worker_;

	// The folder only depends on the web port and the models, so a server that was killed
	// leaves one behind that the next start clears out.
	static ghc::filesystem::path emptyDirectory(const std::string& modelsPath, uint16_t port)
	{
		std::error_code ignored;
		auto dir = ghc::filesystem::temp_directory_path(ignored) / ("omp-modelpack-" + std::to_string(port) + "-" + std::to_string(std::hash<std::string> {}(modelsPath)));
		ghc::filesystem::remove_all(dir, ignored);
		return dir;
	}

	// An older build stays in use while a newer one is being made.
	Status status() const
	{
		if (built_)
		{
			return Status::Ready;
		}
		return entries_.empty() || failed_ ? Status::None : Status::Building;
	}

	void run()
	{
		std::unique_lock<std::mutex> lock(mutex_);
		while (!stop_)
		{
			wake_.wait(lock, [this]
				{
					return stop_.load() || dirty_;
				});
			// The gamemode registers its models in one go. Wait until it has stopped adding.
			size_t seen;
			do
			{
				seen = entries_.size();
				wake_.wait_for(lock, std::chrono::seconds(2), [this]
					{
						return stop_.load();
					});
			} while (!stop_ && entries_.size() != seen);
			if (stop_)
			{
				break;
			}

			dirty_ = false;
			auto entries = entries_;
			auto total = total_;
			auto number = ++builds_;
			lock.unlock();
			auto built = build(std::move(entries), total, number);
			lock.lock();

			if (!built)
			{
				failed_ = !dirty_;
				continue;
			}
			auto old = std::exchange(built_, std::move(built));
			if (old)
			{
				removeFiles(*old);
			}
		}
	}

	void removeFiles(const Built& built) const
	{
		std::error_code ignored;
		for (const auto& segment : built.segments)
		{
			ghc::filesystem::remove(segment.path, ignored);
		}
	}

	std::shared_ptr<const Built> build(std::vector<Entry> entries, uint64_t total, unsigned number)
	{
		uint64_t target = MinSegmentBytes;
		while (target < MaxSegmentBytes && target * SegmentCount < total)
		{
			target *= 2;
		}
		size_t segments = 1;
		uint64_t filled = 0;
		for (auto& entry : entries)
		{
			if (filled >= target)
			{
				++segments;
				filled = 0;
			}
			entry.segment = segments - 1;
			filled += entry.size;
		}

		std::error_code ignored;
		ghc::filesystem::create_directories(dir_, ignored);
		auto built = std::make_shared<Built>();
		for (size_t i = 0; i < segments; ++i)
		{
			auto path = dir_ / (std::to_string(number) + "-" + std::to_string(i) + ".seg");
			built->segments.push_back({ path.string(), 0 });
			if (!deflateSegment(entries, i, built->segments.back()))
			{
				removeFiles(*built);
				return nullptr;
			}
		}

		built->index = "{\"v\":1,\"packs\":[";
		for (size_t i = 0; i < segments; ++i)
		{
			built->index += (i ? "," : "") + std::to_string(built->segments[i].size);
		}
		built->index += "],\"files\":[";
		for (size_t i = 0; i < entries.size(); ++i)
		{
			const auto& entry = entries[i];
			// Cache names are 0x<hex>.dff or .txd, nothing in them needs escaping.
			built->index += std::string(i ? "," : "") + "{\"n\":\"" + entry.cacheName + "\",\"s\":" + std::to_string(entry.size) + ",\"c\":" + std::to_string(entry.crc) + ",\"p\":" + std::to_string(entry.segment) + "}";
		}
		built->index += "]}";
		return built;
	}

	// Writes the files of one segment, compressed, to segment.path. Fails if a file is gone or
	// has changed since it was registered, because its cache name would then be wrong.
	bool deflateSegment(const std::vector<Entry>& entries, size_t number, Segment& segment)
	{
		FILE* out = fopen(segment.path.c_str(), "wb");
		if (!out)
		{
			return false;
		}
		z_stream stream {};
		if (deflateInit(&stream, 6) != Z_OK)
		{
			fclose(out);
			return false;
		}

		unsigned char in[65536];
		unsigned char buffer[65536];
		bool ok = true;
		auto compress = [&](int flush)
		{
			int result;
			do
			{
				stream.next_out = buffer;
				stream.avail_out = sizeof buffer;
				result = deflate(&stream, flush);
				size_t count = sizeof buffer - stream.avail_out;
				segment.size += count;
				ok = ok && fwrite(buffer, 1, count, out) == count;
			} while (stream.avail_out == 0 && ok);
			return result;
		};

		for (const auto& entry : entries)
		{
			if (entry.segment != number)
			{
				continue;
			}
			FILE* file = fopen((modelsPath_ + "/" + entry.diskName).c_str(), "rb");
			if (!file)
			{
				ok = false;
				break;
			}
			uLong crc = crc32(0, Z_NULL, 0);
			uint64_t size = 0;
			size_t count;
			while (ok && !stop_ && (count = fread(in, 1, sizeof in, file)) > 0)
			{
				crc = crc32(crc, in, uInt(count));
				size += count;
				stream.next_in = in;
				stream.avail_in = uInt(count);
				compress(Z_NO_FLUSH);
			}
			fclose(file);
			if (!ok || stop_ || size != entry.size || uint32_t(crc) != entry.crc)
			{
				ok = false;
				break;
			}
		}
		if (ok)
		{
			stream.avail_in = 0;
			ok = compress(Z_FINISH) == Z_STREAM_END && ok;
		}
		deflateEnd(&stream);
		return fclose(out) == 0 && ok;
	}
};
