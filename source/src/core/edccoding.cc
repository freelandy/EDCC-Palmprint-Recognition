// Copyright (c) 2017 Leosocy. All rights reserved.
// Use of this source code is governed by a MIT-style license 
// that can be found in the LICENSE file.

#include "core/edccoding.h"

#include "core/check.h"
#include "core/match.h"
#include "edcc.h"

namespace edcc
{

EDCCoding::EDCCoding(const EDCCoding &rhs)
{
    c_ = rhs.c_.clone();
    cs_ = rhs.cs_.clone();
    magic_key_ = rhs.magic_key_;
    coding_buffer_ = NULL;
    if (rhs.coding_buffer_ != NULL)
    {
        coding_buffer_ = (EDCC_CODING_T*)malloc(rhs.buffer_len());
        memcpy(coding_buffer_, rhs.coding_buffer_, rhs.buffer_len());
    }
}

EDCCoding& EDCCoding::operator =(const EDCCoding &rhs)
{
    if (this != &rhs)
    {
        c_ = rhs.c_.clone();
        cs_ = rhs.cs_.clone();
        magic_key_ = rhs.magic_key_;
        FreeCodingBuffer(&coding_buffer_);
        if (rhs.coding_buffer_ != NULL)
        {
            coding_buffer_ = (EDCC_CODING_T*)malloc(rhs.buffer_len());
            memcpy(coding_buffer_, rhs.coding_buffer_, rhs.buffer_len());
        }
    }
    return *this;
}

EDCCoding::~EDCCoding()
{
    FreeCodingBuffer(&coding_buffer_);
}

Status EDCCoding::Encode(const EDCC_CFG_T &config,
                         size_t buffer_max_len,
                         u_char *coding_buffer,
                         size_t *buffer_size)
{
    if (coding_buffer == NULL)
    {
        *buffer_size = 0;
        return EDCC_NULL_POINTER_ERROR;
    }
    if (coding_buffer_ != NULL)
    {
        if (buffer_max_len < buffer_len())
        {
            EDCC_Log("EDCCoding::encrypt bufMaxLen smaller than the real space occupied!");
            *buffer_size = 0;
            return EDCC_CODING_BUFF_LEN_NOT_ENOUGH;
        }
        memcpy(coding_buffer, coding_buffer_, buffer_len());
        *buffer_size = buffer_len();
        return EDCC_SUCCESS;
    }

    Status s = Encode(config, buffer_size);
    if (s != EDCC_SUCCESS)
    {
        return s;
    }
    if (buffer_max_len < *buffer_size)
    {
        EDCC_Log("EDCCoding::encrypt bufMaxLen smaller than the real space occupied!");
        *buffer_size = 0;
        return EDCC_CODING_BUFF_LEN_NOT_ENOUGH;
    }
    memcpy(coding_buffer, coding_buffer_, *buffer_size);

    return s;
}

Status EDCCoding::Encode(const EDCC_CFG_T &config, size_t *buffer_size)
{
    if (coding_buffer_ != NULL)
    {
        *buffer_size = buffer_len();
        return EDCC_SUCCESS;
    }

    Check checker;
    if (!checker.CheckConfig(config))
    {
        EDCC_Log("EDCCoding::encrypt config error!");
        *buffer_size = 0;
        return EDCC_LOAD_CONFIG_FAIL;
    }
    MallocCodingBuffer(config, &coding_buffer_);
    if (coding_buffer_ == NULL)
    {
        *buffer_size = 0;
        return EDCC_NULL_POINTER_ERROR;
    }
    GenCodingBytes();
    *buffer_size = buffer_len();

    return EDCC_SUCCESS;
}

Status EDCCoding::EncodeToHexString(const EDCC_CFG_T &config, string *hex_str)
{
    assert(hex_str);
    size_t buffer_size = 0;
    Status s = Encode(config, &buffer_size);
    if (s != EDCC_SUCCESS)
    {
        hex_str->clear();
        return s;
    }

    size_t pos = 0;
    stringstream hex_string_stream;
    while (pos < buffer_size)
    {
        char hex_c[3];
        sprintf(hex_c, "%02x", ((unsigned char*)coding_buffer_)[pos]);
        hex_string_stream << hex_c;
        ++pos;
    }
    *hex_str = hex_string_stream.str();

    return EDCC_SUCCESS;
}

Status EDCCoding::Decode(const u_char *coding_buffer)
{
    if (coding_buffer == NULL)
    {
        return EDCC_NULL_POINTER_ERROR;
    }

    Check checker;
    const EDCC_CODING_T *coding = (EDCC_CODING_T*)coding_buffer;
    size_t coding_len = coding->len + sizeof(EDCC_CODING_T);
    int actual_magic_key = 0;
    memcpy(&actual_magic_key,
           coding->data + coding->len - kMagicKeyLen,
           kMagicKeyLen);
    if (actual_magic_key != magic_key_)
    {
        return EDCC_CODING_INVALID;
    }

    MallocCodingBuffer(coding->cfg, &coding_buffer_);
    if (coding_buffer_ == NULL)
    {
        EDCC_Log("EDCCoding::decrypt failed!");
        return EDCC_NULL_POINTER_ERROR;
    }
    memcpy(coding_buffer_, coding, coding_len);

    return checker.CheckCoding(*this) ? EDCC_SUCCESS : EDCC_CODING_INVALID;
}

Status EDCCoding::DecodeFromHexString(const string &hex_str)
{
    size_t coding_len = hex_str.length() / 2;
    CHECK_EQ_RETURN(coding_len, 0, false);
    u_char* coding_buffer = (unsigned char*)malloc(sizeof(unsigned char) * coding_len);
    CHECK_POINTER_NULL_RETURN(coding_buffer, EDCC_NULL_POINTER_ERROR);
    memset(coding_buffer, 0, sizeof(unsigned char) * coding_len);

    for (size_t i = 0; i < coding_len; ++i)
    {
        string hex_c = hex_str.substr(i * 2, 2);
        sscanf(hex_c.c_str(), "%02x", coding_buffer + i);
    }
    Status s = Decode(coding_buffer);
    free(coding_buffer);

    return s;
}

void EDCCoding::GenCodingBytes()
{
    CHECK_POINTER_NULL_RETURN_VOID(coding_buffer_);
    memset(coding_buffer_->data, 0, coding_buffer_->len);

    size_t offset = 0;
    int counter = 0;
    for (int h = 0; h < c_.rows; ++h)
    {
        for (int w = 0; w < c_.cols; ++w)
        {
            ++counter;
            unsigned char codingC = c_.at<char>(h, w);
            if (counter % 2 != 0)
            {
                codingC <<= 4;
                codingC &= 0xf0;
            }
            *(coding_buffer_->data + offset) |= codingC;
            if (counter == c_.rows*c_.cols
                || counter % 2 == 0)
            {
                ++offset;
            }
        }
    }

    counter = 0;
    for (int h = 0; h < cs_.rows; ++h)
    {
        for (int w = 0; w < cs_.cols; ++w)
        {
            unsigned char codingCs = cs_.at<char>(h, w);
            *(coding_buffer_->data + offset) |= (codingCs << (7 - (counter % 8)));
            ++counter;
            if (counter == cs_.rows*cs_.cols
                || counter % 8 == 0)
            {
                ++offset;
            }
        }
    }

    memcpy(coding_buffer_->data + offset, &magic_key_, kMagicKeyLen);
}

void EDCCoding::MallocCodingBuffer(const EDCC_CFG_T &config, EDCC_CODING_T **buffer)
{
    FreeCodingBuffer(buffer);

    size_t image_size = config.imageSizeW*config.imageSizeH;
    size_t c_len = (size_t)ceil(image_size / 2.0);
    size_t cs_len = (size_t)ceil(image_size / 8.0);
    size_t buffer_len = sizeof(EDCC_CODING_T)+c_len+cs_len+kMagicKeyLen;

    *buffer = (EDCC_CODING_T *)malloc(buffer_len);
    CHECK_POINTER_NULL_RETURN_VOID(*buffer);
    memset(*buffer, 0, buffer_len);

    memcpy(&((*buffer)->cfg), &config, sizeof(EDCC_CFG_T));
    (*buffer)->len = static_cast<u_int>(buffer_len - sizeof(EDCC_CODING_T));
}

void EDCCoding::FreeCodingBuffer(EDCC_CODING_T **buffer)
{
    if (*buffer != NULL)
    {
        free(*buffer);
        *buffer = NULL;
    }
}

} // namespace edcc