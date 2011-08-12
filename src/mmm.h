#ifndef MMM_H
#define MMM_H

class MMM {
public:
    MMM();

    void* new(unsigned int size);

private:
    unsigned int  _size;
    unsigned int  _pos;
    char         *_buffer;
};


#endif // MMM_H
