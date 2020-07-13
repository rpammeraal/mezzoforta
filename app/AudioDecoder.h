#ifndef AUDIODECODER_H
#define AUDIODECODER_H

#include <fcntl.h>
#include <portaudio.h>

#include <QFile>
#include <QString>
#include <QByteArray>
#include <QThread>

#include "SBIDSong.h"

class AudioDecoderReader;

///
/// \brief The AudioDecoder class
///
/// This class is a parent class for all codecs. Given a fileName, data can
/// be retrieved with getSamples() in subsequent calls. SetPosition() may be
/// used to set the 'current' position somewhere else in the underlying file.
///
class AudioDecoder : public QObject
{
    Q_OBJECT

public:
    virtual ~AudioDecoder();
    quint64 getSamples(void* buffer, quint64 sampleCount);
    quint64 setPosition(qint64 position);
    inline quint64 getIndex() const { return _index; }

    //	Meta data
    inline quint16 bitsPerSample() const { return _bitsPerSample; }
    inline QString error() const { return _error; }
    inline quint64 lengthInBytes() const { return _length; }
    inline quint64 lengthInMs() const { return index2MS(_length); }
    inline quint16 numChannels() const { return _numChannels; }
    inline PaSampleFormat sampleFormat() const { return _sampleFormat; }
    inline quint32 sampleRate() const { return _sampleRate; }
    inline int bytesPerStereoSample() const { return bitsPerSample()*2/8; }

#ifdef Q_OS_WIN
	static QString convertToWindowsPath(const QString& path);
#endif

    //	Conversion
    quint64 bytesToSamples(quint64 bytes) const { return (_numChannels*_bitsPerSample==0)?0:(bytes * 8)/(_numChannels * _bitsPerSample); }
    quint64 samplesToBytes(quint64 samples) const { return (samples * _numChannels * _bitsPerSample)/8 ;}
    quint64 ms2Index(quint64 ms) const { return (_sampleRate * ms * _bitsPerSample * _numChannels * 1.0)/(8 * 1000.0); }
    /*

             44100sr * 1000ms * 16bps *2ch
    1000ms = ----------------------------- = 176400
                8b/b * 1000ms/s
     */

    quint64 index2MS(quint64 index) const { return (_sampleRate*_numChannels*_bitsPerSample==0)?0:(index * 8.0 * 1000)/(_sampleRate * _numChannels * _bitsPerSample); }
    /*
             176400 * 8b/b * 1000ms   1411200000
    176400 = ---------------------- = ---------- = 1000ms
             44100sr * 2ch * 16bps    1411200
     */

    QString _fileName;

protected:
    friend class AudioDecoderFactory;
    friend class AudioDecoderFlacReader;
    friend class AudioDecoderMP3Reader;
    friend class AudioDecoderOggVorbisReader;
    friend class AudioDecoderWaveReader;

    //	Stream parameters
    quint16             _bitsPerSample;
    quint64             _length;	//	in bytes
    quint16             _numChannels;
    quint32             _sampleRate;
    PaSampleFormat      _sampleFormat;
    char*               _stream;	//	pointer to stream in memory

    //	Header
    //	SBIDSongPtr         _headerPtr;

    //	Other
    QFile*              _file;
    quint64             _index;	//	pointer in bytes
    quint64             _maxScrollableIndex;	//	pointer in bytes to end of _stream read so far by *Reader class
    QString             _error;
    QThread             _workerThread;
//    AudioDecoderReader* _adr;
    FILE* 				_fp;

    AudioDecoder();

    static bool supportFileExtension(const QString& extension) ;

signals:
    void startBackfill();

private:

    void _init();
    virtual void _exit();

};

#endif // AUDIODECODER_H
