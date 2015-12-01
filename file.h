//  $Id: file.h,v 1.6 1999/11/26 10:14:09 cisc Exp $

#ifndef FMGON_FILE_H
#define FMGON_FILE_H

// ---------------------------------------------------------------------------

class FileIO {
public:
    FileIO() : m_fp(NULL) { }

    FileIO(const char* filename, const char *mode) {
        Open(filename, mode);
    }

    virtual ~FileIO() {
        Close();
    }

    bool Open(const char *filename, const char *mode) {
        m_fp = fopen(filename, mode);
        return (m_fp != NULL);
    }

    void Close() {
        if (m_fp != NULL) {
            fclose(m_fp);
            m_fp = NULL;
        }
    }

    bool Read(void *dest, size_t len) {
        return (fread(dest, len, 1, m_fp) == 1);
    }

    bool Seek(long offset, int ptrname) {
        return (fseek(m_fp, offset, ptrname) == 0);
    }

    bool HasError() const {
        return (ferror(m_fp) == 0);
    }

protected:
    FILE *m_fp;

private:
    FileIO(const FileIO&);
    const FileIO& operator=(const FileIO&);
}; // class FileIO

#endif  // ndef FMGON_FILE_H
