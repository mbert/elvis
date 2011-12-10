/* gwimage.c -- Copyright 2003 by Steve Kirkendall */

#define CHAR    Char
#include "elvis.h" 
#undef CHAR
#undef ELVBOOL 
#ifdef FEATURE_IMAGE
# include <windows.h>

/* read the next string from an XPM file */
static char *getstring(fp)
    FILE	*fp;
{
    int	ch;
    int	i;
    static char *buf;
    static int  bufsize = 0;

    /* skip to start of string */
    while ((ch = getc(fp)) != EOF && ch != '"')
    {
    }
    if (ch == EOF)
	return NULL;

    /* if first time, then allocate a buffer */
    if (!buf)
    {
	bufsize = 100;
	buf = (char *)malloc(100);
    }

    /* collect characters of the string */
    for (i = 0; (ch = getc(fp)) != '"'; )
    {
	/* allow backslash-quoted chars (but not octal or ctrl chars) */
	if (ch == '\\')
	    ch = getc(fp);

	/* if EOF then return nothing */
	if (ch == EOF)
	    return NULL;

	/* increase the buffer size if necessary */
	if (i + 1 >= bufsize)
	{
	    bufsize *= 2;
	    buf = realloc(buf, bufsize);
	}

	/* store the character */
	buf[i++] = ch;
    }

    /* mark the end of the string, and return it */
    buf[i] = '\0';
    return buf;
}

/* Load an image from an XPM file into a HBITMAP.  Optionally tint it.  Returns
 * the bitmap's handle if successful, or 0 if failure (without an error msg)
 */
HBITMAP gw_load_xpm(filename, tint, average, mask)
    char	*filename;	/* name of *.xpm file to load */
    long	tint;		/* -1 for pure image, or a COLORREF to tint */
    long	*average;	/* average color return (may be NULL) */
    HBITMAP	*mask;		/* mask return (may be NULL) */
{
    FILE	*fp;
    char	*line;
    char	*name;
    int		width, height, colors, pixelchars;
    int		ch, ch2;
    int		ci, row, col;
    long	palette[95*95];
    char	transparent[95*95];
    unsigned char rgb[3];
    HDC		hdc = NULL;
    HBITMAP	bitmap = NULL;
    long	avg, avg1, avg2, avg3;

    /* open the file */
    fp = fopen(filename, "r");
    if (!fp)
    {
	name = (char *)malloc(strlen(filename) + 8);
	strcpy(name, "themes/");
	strcat(name, filename);
	line = iopath(tochar8(o_elvispath), name, ElvFalse);
	free(name);
	if (!line) goto Fail;
	fp = fopen(line, "r");
    }
    if (!fp) goto Fail;

    /* read & parse the header */
    line = getstring(fp);
    if (!line || 4 != sscanf(line, "%d %d %d %d",
		    &width, &height, &colors, &pixelchars))
	goto Fail;

    /* for each color table entry... */
    for (ci = 0; ci < colors; ci++)
    {
	/* read the color table entry */
	line = getstring(fp);
	if (!line) goto Fail;

	/* parse the color code */
	ch = *line++;
	if (!ch) goto Fail;
	ch -= 32;
	if (pixelchars == 2)
	{
	    ch2 = *line++;
	    if (!ch2) goto Fail;
	    ch2 -= 32;
	    ch += 95 * ch2;
	}

	/* parse the color tag and name */
	while (elvspace(*line))
	    line++;
	if (!*line) goto Fail;
	if (*line == 'c')
	{
	    /* skip to the start of the color name */
	    line++;
	    while (elvspace(*line))
		line++;
	    if (!*line) goto Fail;

	    /* convert the color name to a code */
	    if (!(guiwin32.color)(0,line, ElvFalse, &palette[ch], rgb))
		goto Fail;

	    /* tint the code, if necessary */
	    if (tint >= 0)
		palette[ch] = ((palette[ch] & 0xfefefe)
			 + (tint & 0xfefefe)) >> 1;

	    /* it isn't transparent */
	    transparent[ch] = 0;
	}
	else /* it must be transparent */
	{
	    if (tint >= 0)
		palette[ch] = tint;
	    else
		palette[ch] = 0x0;
	    transparent[ch] = 1;
	}
    }

    /* create a device context for the display */
    hdc = CreateCompatibleDC(NULL);
    if (!hdc) goto Fail;
	SetMapMode(hdc, MM_TEXT);

    /* allocate the bitmap */
    bitmap = CreateBitmap(width, height, 1, GetDeviceCaps(hdc, BITSPIXEL), 
	malloc((height * (GetDeviceCaps(hdc, BITSPIXEL) * width / 8) + 1 & ~0x1)));
    if (!bitmap) goto Fail;

    /* if a mask is desired, then allocate it too  */
    if (mask)
    {
	    *mask = CreateBitmap(width, height, 1, GetDeviceCaps(hdc, BITSPIXEL), 
		malloc((height * (GetDeviceCaps(hdc, BITSPIXEL) * width / 8) + 1 & ~0x1)));
	    if (!*mask) goto Fail;
    }

    /* use the bitmap in this context */
    if (SelectObject(hdc, bitmap) == NULL)
	goto Fail;

    /* for each row of the image... */
    for (row = 0; row < height; row++)
    {
	/* fetch the row */
	line = getstring(fp);
	if (!line) goto Fail;

	/* for each column in the row */
	for (col = 0; col < width; col++)
	{
	    /* parse the color code */
	    ch = *line++;
	    if (!ch) goto Fail;
	    ch -= 32;
	    if (pixelchars == 2)
	    {
		ch2 = *line++;
		if (!ch2) goto Fail;
		ch2 -= 32;
		ch += 95 * ch2;
	    }

	    /* draw the mask, if necessary */
	    if (mask)
	    {
		SelectObject(hdc, *mask);
		if (transparent[ch])
		    SetPixel(hdc, col, row, (COLORREF)0xffffff);
		else
		    SetPixel(hdc, col, row, (COLORREF)0x000000);
		SelectObject(hdc, bitmap);
	    }

	    /* draw this pixel in the correct color */
	    SetPixel(hdc, col, row, (COLORREF)(palette[ch]));
	}
    }

    /* find the average color from the image */
    if (average)
    {
	avg1 = avg2 = avg3 = 0;
	for (row = height / 8; row < height; row += height / 4)
	{
	    for (col = width / 8; col < width; col += width / 4)
	    {
		avg = GetPixel(hdc, col, row);
		avg1 += avg & 0x0000ff;
		avg2 += avg & 0x00ff00;
		avg3 += avg & 0xff0000;
	    }
	}
	avg = ((avg1&0x0000ff0) + (avg2&0x00ff000) + (avg3&0xff00000)) >> 4;
	*average = avg;
    }

    /* clean up */
    DeleteDC(hdc);
    fclose(fp);
    return bitmap;

Fail:
    if (hdc)
	DeleteDC(hdc);
    if (bitmap)
	DeleteObject(bitmap);
    if (fp != NULL)
	fclose(fp);
    return NULL;
}

/* Free a bitmap that was created by gw_load_xpm().  If you also created
 * a mask bitmap at the same time, then you must call this function separately
 * for both bitmaps.
 */
void gw_unload_xpm(bitmap)
    HBITMAP	bitmap;	/* the bitmap to be freed */
{
    BITMAP	bm;

    /* get info about the bitmap */
    GetObject(bitmap, sizeof(BITMAP), &bm);

    /* free the image memory */
    free(bm.bmBits);

    /* free the bitmap itself */
    DeleteObject(bitmap);
}

/* Erase part of a drawing context by copying sections of a bitmap image
 * into it.
 */
void gw_erase_rect(hdc, rect, bitmap, scrolled)
    HDC		hdc;	/* the destination context; what to clear */
    RECT	*rect;	/* area to erase within the destination */
    HBITMAP	bitmap;	/* the image to use for clearing */
    long	scrolled;/* number of pixels scrolled vertically */
{
    HBITMAP	old;
    BITMAP	bm;
    HDC		bmphdc;
    int		x, y, h, w, ret;
    int		base;

    /* get the bitmap's size */
    GetObject(bitmap, sizeof(BITMAP), &bm);

    /* create a drawing context for the source bitmap */
    bmphdc = CreateCompatibleDC(hdc);
    SetMapMode(bmphdc, MM_TEXT);
    old = SelectObject(bmphdc, bitmap);

    /* choose a height and base for the first row */
    scrolled += rect->top;
    if (scrolled < 0)
	h = (int)(-scrolled % bm.bmHeight);
    else
	h = bm.bmHeight - (int)(scrolled % bm.bmHeight);
    base = bm.bmHeight - h;

    /* for each row or partial row... */
    for (y = rect->top; y < rect->bottom; y += h, h = bm.bmHeight, base = 0)
    {
	/* we may need to reduce the height of the bottom row */
	if (y + h > rect->bottom)
	    h = rect->bottom - y;

	/* for each column or partial column... */
	for (x = rect->left; x < rect->right; x += w)
	{
	    w = bm.bmWidth - x % bm.bmWidth;
	    if (x + w > rect->right)
		w = rect->right - x;

	    /* copy the source image into this row&column of the destination */
	    ret = BitBlt(hdc, x, y, w, h, bmphdc,
	    		 x % bm.bmWidth, base, SRCCOPY);
	}
    }

    /* free the drawing context */
    SelectObject(bmphdc, old);
    DeleteDC(bmphdc);
}
#endif /* FEATURE_IMAGE */
/* ex:set sw=4: */
