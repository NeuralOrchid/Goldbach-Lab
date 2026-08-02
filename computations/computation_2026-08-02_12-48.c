// Created by ChatGPT (OpenAI)
// Date: 2026-08-02
//
// ---------------------------
// computation_2026-08-02_12-48.c
// Build: gcc -O3 -std=c11 computation_2026-08-02_12-48.c -o divpng
// Usage: divpng 2048 divisibility.png

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define RAW_BLOCK_MAX 65535u
#define IDAT_BUF_CAP   (1u << 20)   // 1 MiB IDAT chunk buffer

static uint32_t crc_table[256];
static int crc_table_ready = 0;

static void init_crc32_table(void) {
    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t c = i;
        for (int k = 0; k < 8; ++k) {
            c = (c & 1u) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
        }
        crc_table[i] = c;
    }
    crc_table_ready = 1;
}

static uint32_t crc32_update(uint32_t crc, const unsigned char *buf, size_t len) {
    if (!crc_table_ready) init_crc32_table();
    while (len--) {
        crc = crc_table[(crc ^ *buf++) & 0xFFu] ^ (crc >> 8);
    }
    return crc;
}

static uint32_t adler32_update(uint32_t adler, const unsigned char *buf, size_t len) {
    const uint32_t MOD = 65521u;
    uint32_t s1 = adler & 0xFFFFu;
    uint32_t s2 = (adler >> 16) & 0xFFFFu;

    while (len) {
        size_t t = len > 5552 ? 5552 : len;
        len -= t;
        for (size_t i = 0; i < t; ++i) {
            s1 += *buf++;
            s2 += s1;
        }
        s1 %= MOD;
        s2 %= MOD;
    }

    return (s2 << 16) | s1;
}

static void write_be32(FILE *f, uint32_t v) {
    unsigned char b[4];
    b[0] = (unsigned char)((v >> 24) & 0xFFu);
    b[1] = (unsigned char)((v >> 16) & 0xFFu);
    b[2] = (unsigned char)((v >> 8) & 0xFFu);
    b[3] = (unsigned char)(v & 0xFFu);
    fwrite(b, 1, 4, f);
}

static void write_chunk(FILE *f, const char type[4], const unsigned char *data, uint32_t len) {
    write_be32(f, len);
    fwrite(type, 1, 4, f);
    if (len && data) {
        fwrite(data, 1, len, f);
    }

    uint32_t crc = 0xFFFFFFFFu;
    crc = crc32_update(crc, (const unsigned char *)type, 4);
    if (len && data) {
        crc = crc32_update(crc, data, len);
    }
    crc ^= 0xFFFFFFFFu;
    write_be32(f, crc);
}

typedef struct {
    FILE *f;
    unsigned char *idat;
    size_t idat_len;
    size_t idat_cap;
    unsigned char raw[RAW_BLOCK_MAX];
    size_t raw_len;
    uint32_t adler;
} PngWriter;

static void idat_flush(PngWriter *w) {
    if (w->idat_len == 0) return;
    write_chunk(w->f, "IDAT", w->idat, (uint32_t)w->idat_len);
    w->idat_len = 0;
}

static void idat_put_byte(PngWriter *w, unsigned char b) {
    if (w->idat_len == w->idat_cap) {
        idat_flush(w);
    }
    w->idat[w->idat_len++] = b;
}

static void idat_put_bytes(PngWriter *w, const unsigned char *data, size_t len) {
    while (len) {
        size_t space = w->idat_cap - w->idat_len;
        if (space == 0) {
            idat_flush(w);
            space = w->idat_cap;
        }
        size_t take = len < space ? len : space;
        memcpy(w->idat + w->idat_len, data, take);
        w->idat_len += take;
        data += take;
        len -= take;
    }
}

// Emits one stored DEFLATE block into the zlib stream.
// If final != 0, this is the last block (even if raw_len == 0).
static void raw_flush(PngWriter *w, int final) {
    if (!final && w->raw_len == 0) return;

    // Need space for: 1 block header + 4 LEN/NLEN + raw_len bytes
    size_t need = 1u + 4u + w->raw_len;
    if (w->idat_cap - w->idat_len < need) {
        idat_flush(w);
    }

    idat_put_byte(w, (unsigned char)(final ? 0x01u : 0x00u)); // BFINAL=1/0, BTYPE=00

    uint16_t len = (uint16_t)w->raw_len;
    uint16_t nlen = (uint16_t)(0xFFFFu - len);

    idat_put_byte(w, (unsigned char)(len & 0xFFu));
    idat_put_byte(w, (unsigned char)((len >> 8) & 0xFFu));
    idat_put_byte(w, (unsigned char)(nlen & 0xFFu));
    idat_put_byte(w, (unsigned char)((nlen >> 8) & 0xFFu));

    idat_put_bytes(w, w->raw, w->raw_len);
    w->raw_len = 0;
}

static void raw_write(PngWriter *w, const unsigned char *data, size_t len) {
    while (len) {
        size_t space = RAW_BLOCK_MAX - w->raw_len;
        if (space == 0) {
            raw_flush(w, 0);
            space = RAW_BLOCK_MAX;
        }
        size_t take = len < space ? len : space;
        memcpy(w->raw + w->raw_len, data, take);
        w->raw_len += take;
        data += take;
        len -= take;
        if (w->raw_len == RAW_BLOCK_MAX) {
            raw_flush(w, 0);
        }
    }
}

int main(int argc, char **argv) {
    size_t n = 2048;
    const char *out = "divisibility.png";

    if (argc >= 2) {
        n = (size_t)strtoull(argv[1], NULL, 10);
    }
    if (argc >= 3) {
        out = argv[2];
    }

    if (n == 0) {
        fprintf(stderr, "n must be >= 1\n");
        return 1;
    }

    FILE *f = fopen(out, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }

    unsigned char *row = (unsigned char *)malloc(n);
    unsigned char *idat = (unsigned char *)malloc(IDAT_BUF_CAP);
    if (!row || !idat) {
        fprintf(stderr, "memory allocation failed\n");
        fclose(f);
        free(row);
        free(idat);
        return 1;
    }

    // PNG signature
    static const unsigned char sig[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    fwrite(sig, 1, 8, f);

    // IHDR
    unsigned char ihdr[13];
    ihdr[0]  = (unsigned char)((n >> 24) & 0xFFu);
    ihdr[1]  = (unsigned char)((n >> 16) & 0xFFu);
    ihdr[2]  = (unsigned char)((n >> 8) & 0xFFu);
    ihdr[3]  = (unsigned char)(n & 0xFFu);
    ihdr[4]  = (unsigned char)((n >> 24) & 0xFFu);
    ihdr[5]  = (unsigned char)((n >> 16) & 0xFFu);
    ihdr[6]  = (unsigned char)((n >> 8) & 0xFFu);
    ihdr[7]  = (unsigned char)(n & 0xFFu);
    ihdr[8]  = 8;   // bit depth
    ihdr[9]  = 0;   // grayscale
    ihdr[10] = 0;   // compression method
    ihdr[11] = 0;   // filter method
    ihdr[12] = 0;   // no interlace
    write_chunk(f, "IHDR", ihdr, 13);

    PngWriter w;
    w.f = f;
    w.idat = idat;
    w.idat_len = 0;
    w.idat_cap = IDAT_BUF_CAP;
    w.raw_len = 0;
    w.adler = 1u;

    // zlib header for "no compression" stream
    idat_put_byte(&w, 0x78);
    idat_put_byte(&w, 0x01);

    for (size_t i = 1; i <= n; ++i) {
        memset(row, 255, n);

        // black at multiples of i: j = i, 2i, 3i, ...
        for (size_t k = i - 1; k < n; k += i) {
            row[k] = 0;
        }

        unsigned char filter = 0; // PNG filter type 0
        w.adler = adler32_update(w.adler, &filter, 1);
        raw_write(&w, &filter, 1);

        w.adler = adler32_update(w.adler, row, n);
        raw_write(&w, row, n);
    }

    // Final DEFLATE stored block, then Adler-32
    raw_flush(&w, 1);

    unsigned char adler_bytes[4];
    adler_bytes[0] = (unsigned char)((w.adler >> 24) & 0xFFu);
    adler_bytes[1] = (unsigned char)((w.adler >> 16) & 0xFFu);
    adler_bytes[2] = (unsigned char)((w.adler >> 8) & 0xFFu);
    adler_bytes[3] = (unsigned char)(w.adler & 0xFFu);
    idat_put_bytes(&w, adler_bytes, 4);

    idat_flush(&w);

    // IEND
    write_chunk(f, "IEND", NULL, 0);

    fclose(f);
    free(row);
    free(idat);

    return 0;
}