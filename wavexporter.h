#ifndef WAVEXPORTER_H
#define WAVEXPORTER_H

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>

#include <QAudioFormat>
#include <QByteArray>
#include <QFile>
#include <QString>

#include "bytebeatgenerator.h"

class WavExporter final {
public:
	static bool exportUnsigned(const QString &formula, const QString &filePath,
							   int sampleRate, qint64 durationSeconds,
							   QString *errorMessage = nullptr)
	{
		return exportFormula(formula, filePath, sampleRate, durationSeconds,
							 BYTEBEAT_UNSIGNED, errorMessage);
	}

	static bool exportSigned(const QString &formula, const QString &filePath,
							 int sampleRate, qint64 durationSeconds,
							 QString *errorMessage = nullptr)
	{
		return exportFormula(formula, filePath, sampleRate, durationSeconds,
							 BYTEBEAT_SIGNED, errorMessage);
	}

private:
	static bool writeUInt16(std::FILE *file, std::uint16_t value)
	{
		const unsigned char bytes[] = {
			static_cast<unsigned char>(value & 0xFF),
			static_cast<unsigned char>((value >> 8) & 0xFF)
		};
		return std::fwrite(bytes, sizeof(bytes), 1, file) == 1;
	}

	static bool writeUInt32(std::FILE *file, std::uint32_t value)
	{
		const unsigned char bytes[] = {
			static_cast<unsigned char>(value & 0xFF),
			static_cast<unsigned char>((value >> 8) & 0xFF),
			static_cast<unsigned char>((value >> 16) & 0xFF),
			static_cast<unsigned char>((value >> 24) & 0xFF)
		};
		return std::fwrite(bytes, sizeof(bytes), 1, file) == 1;
	}

	static bool writeFourCC(std::FILE *file, const char *value)
	{
		return std::fwrite(value, 1, 4, file) == 4;
	}

	static bool exportFormula(const QString &formula, const QString &filePath,
							  int sampleRate, qint64 durationSeconds,
							  BytebeatType type, QString *errorMessage)
	{
		const std::uint64_t maxDataSize = std::numeric_limits<std::uint32_t>::max() - 36ULL;
		if (sampleRate <= 0 || durationSeconds <= 0 ||
			static_cast<std::uint64_t>(durationSeconds) > maxDataSize /
				(static_cast<std::uint64_t>(sampleRate) * 2ULL)) {
			if (errorMessage) {
				*errorMessage = "The sample rate or duration is outside the supported WAV limits.";
			}
			return false;
		}

		const std::uint32_t dataSize = static_cast<std::uint32_t>(
			static_cast<std::uint64_t>(sampleRate) *
			static_cast<std::uint64_t>(durationSeconds) * 2ULL);
		const std::uint32_t riffSize = dataSize + 36U;

		BytebeatDevice generator;
		if (!generator.setFormula(formula)) {
			if (errorMessage) {
				*errorMessage = "The bytebeat formula is invalid.";
			}
			return false;
		}

		QAudioFormat format;
		format.setSampleRate(sampleRate);
		format.setChannelCount(1);
		format.setSampleFormat(QAudioFormat::Int16);
		generator.setAudioFormat(format);
		generator.setBytebeatType(type);
		if (!generator.open(QIODevice::ReadOnly)) {
			if (errorMessage) {
				*errorMessage = "Could not initialize the bytebeat audio stream.";
			}
			return false;
		}

		const QByteArray encodedPath = QFile::encodeName(filePath);
		std::FILE *file = std::fopen(encodedPath.constData(), "wb");
		if (!file) {
			if (errorMessage) {
				*errorMessage = "Could not open the output file for writing.";
			}
			return false;
		}

		const std::uint32_t byteRate = static_cast<std::uint32_t>(sampleRate) * 2U;
		const bool headerWritten =
			writeFourCC(file, "RIFF") && writeUInt32(file, riffSize) &&
			writeFourCC(file, "WAVE") && writeFourCC(file, "fmt ") &&
			writeUInt32(file, 16U) && writeUInt16(file, 1U) &&
			writeUInt16(file, 1U) && writeUInt32(file, static_cast<std::uint32_t>(sampleRate)) &&
			writeUInt32(file, byteRate) && writeUInt16(file, 2U) &&
			writeUInt16(file, 16U) && writeFourCC(file, "data") &&
			writeUInt32(file, dataSize);

		const auto failExport = [&](const char *message) {
			std::fclose(file);
			std::remove(encodedPath.constData());
			if (errorMessage) {
				*errorMessage = QString::fromLatin1(message);
			}
			return false;
		};

		if (!headerWritten) {
			return failExport("Could not write the WAV header.");
		}

		std::array<unsigned char, 8192> buffer{};
		std::uint32_t bytesRemaining = dataSize;
		while (bytesRemaining > 0) {
			const qint64 requested = qMin<qint64>(buffer.size(), bytesRemaining);
			const qint64 bytesRead = generator.read(
				reinterpret_cast<char *>(buffer.data()), requested);
			if (bytesRead != requested) {
				return failExport("Could not generate all requested audio samples.");
			}

			for (qint64 offset = 0; offset < bytesRead; offset += 2) {
				std::uint16_t sample;
				std::memcpy(&sample, buffer.data() + offset, sizeof(sample));
				buffer[offset] = static_cast<unsigned char>(sample & 0xFF);
				buffer[offset + 1] = static_cast<unsigned char>((sample >> 8) & 0xFF);
			}

			if (std::fwrite(buffer.data(), 1, static_cast<std::size_t>(bytesRead), file) !=
				static_cast<std::size_t>(bytesRead)) {
				return failExport("Could not write the WAV audio data.");
			}
			bytesRemaining -= static_cast<std::uint32_t>(bytesRead);
		}

		if (std::fclose(file) != 0) {
			std::remove(encodedPath.constData());
			if (errorMessage) {
				*errorMessage = "Could not finish writing the WAV file.";
			}
			return false;
		}
		if (errorMessage) {
			errorMessage->clear();
		}
		return true;
	}
};


#endif // WAVEXPORTER_H