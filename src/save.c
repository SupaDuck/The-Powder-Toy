#include <bzlib.h>
#include <math.h>
#include "defines.h"
#include "powder.h"
#include "save.h"
#include "gravity.h"
#include "BSON.h"

int saveversion;
//Pop
pixel *prerender_save(void *save, int size, int *width, int *height)
{
	unsigned char * saveData = save;
	if (size<16)
	{
		return NULL;
	}
	if(saveData[0] == 'O' && saveData[1] == 'P' && saveData[2] == 'S')
	{
		return prerender_save_OPS(save, size, width, height);
	}
	else if((saveData[0]==0x66 && saveData[1]==0x75 && saveData[2]==0x43) || (saveData[0]==0x50 && saveData[1]==0x53 && saveData[2]==0x76))
	{
		return prerender_save_PSv(save, size, width, height);
	}
	return NULL;
}

void *build_save(int *size, int orig_x0, int orig_y0, int orig_w, int orig_h, unsigned char bmap[YRES/CELL][XRES/CELL], float vx[YRES/CELL][XRES/CELL], float vy[YRES/CELL][XRES/CELL], float pv[YRES/CELL][XRES/CELL], float fvx[YRES/CELL][XRES/CELL], float fvy[YRES/CELL][XRES/CELL], sign signs[MAXSIGNS], void* partsptr)
{
	if (save_as == 1 || save_as == 5) //Beta doesn't have PSv format & Release don't have OPS format
		return NULL;

	if (check_save(save_as%3, orig_x0, orig_y0, orig_w, orig_h))
		return NULL;
	if (save_as%3 == 1) //Beta
		saveversion = BETA_VERSION;
	else if (save_as%3 == 2) //Release
		saveversion = RELEASE_VERSION;
	else //Mod
		saveversion = MOD_SAVE_VERSION+192;

	if (save_as >= 3)
		return build_save_OPS(size, orig_x0, orig_y0, orig_w, orig_h, bmap, vx, vy, pv, fvx, fvy, signs, partsptr);
	else
		return build_save_PSv(size, orig_x0, orig_y0, orig_w, orig_h, bmap, fvx, fvy, signs, partsptr);
}

int parse_save(void *save, int size, int replace, int x0, int y0, unsigned char bmap[YRES/CELL][XRES/CELL], float vx[YRES/CELL][XRES/CELL], float vy[YRES/CELL][XRES/CELL], float pv[YRES/CELL][XRES/CELL], float fvx[YRES/CELL][XRES/CELL], float fvy[YRES/CELL][XRES/CELL], sign signs[MAXSIGNS], void* partsptr, unsigned pmap[YRES][XRES])
{
	unsigned char * saveData = save;
	if (size<16)
	{
		return 1;
	}
	if(saveData[0] == 'O' && saveData[1] == 'P' && saveData[2] == 'S')
	{
		return parse_save_OPS(save, size, replace, x0, y0, bmap, vx, vy, pv, fvx, fvy, signs, partsptr, pmap);
	}
	else if((saveData[0]==0x66 && saveData[1]==0x75 && saveData[2]==0x43) || (saveData[0]==0x50 && saveData[1]==0x53 && saveData[2]==0x76))
	{
		return parse_save_PSv(save, size, replace, x0, y0, bmap, fvx, fvy, signs, partsptr, pmap);
	}
	return 1;
}

int invalid_element(int save_as, int el)
{
	if (save_as > 0 && (el >= PT_NORMAL_NUM || ptypes[el].enabled == 0)) //Check for mod/disabled elements
		return 1;
	//if (save_as > 1 && (el == PT_ELEC || el == PT_FIGH || el == PT_ACEL || el == PT_DCEL || el == PT_BANG || el == PT_IGNT)) //Check for beta elements (none right now)
	//	return 1;
	return 0;
}

//checks all elements and ctypes/tmps of certain elements to make sure there are no mod/beta elements in a save or stamp
int check_save(int save_as, int orig_x0, int orig_y0, int orig_w, int orig_h)
{
	int i, x0, y0, w, h, bx0=orig_x0/CELL, by0=orig_y0/CELL, bw=(orig_w+orig_x0-bx0*CELL+CELL-1)/CELL, bh=(orig_h+orig_y0-by0*CELL+CELL-1)/CELL;
	x0 = bx0*CELL;
	y0 = by0*CELL;
	w  = bw *CELL;
	h  = bh *CELL;

	for (i=0; i<NPART; i++)
	{
		if ((int)(parts[i].x+.5f) > x0 && (int)(parts[i].x+.5f) < x0+w && (int)(parts[i].y+.5f) > y0 && (int)(parts[i].y+.5f) < y0+h)
		{
			if (invalid_element(save_as,parts[i].type))
			{
				char errortext[256] = "";
				sprintf(errortext,"Found %s at X:%i Y:%i, cannot save",ptypes[parts[i].type].name,(int)(parts[i].x+.5),(int)(parts[i].y+.5));
				error_ui(vid_buf,0,errortext);
				return 1;
			}
			if ((parts[i].type == PT_CLNE || parts[i].type == PT_PCLN || parts[i].type == PT_BCLN || parts[i].type == PT_PBCN || parts[i].type == PT_STOR || parts[i].type == PT_CONV || parts[i].type == PT_STKM || parts[i].type == PT_STKM2 || parts[i].type == PT_FIGH || parts[i].type == PT_LAVA) && invalid_element(save_as,parts[i].ctype))
			{
				char errortext[256] = "";
				sprintf(errortext,"Found %s at X:%i Y:%i, cannot save",ptypes[parts[i].ctype].name,(int)(parts[i].x+.5),(int)(parts[i].y+.5));
				error_ui(vid_buf,0,errortext);
				return 1;
			}
			if (parts[i].type == PT_PIPE && invalid_element(save_as,parts[i].tmp))
			{
				char errortext[256] = "";
				sprintf(errortext,"Found %s at X:%i Y:%i, cannot save",ptypes[parts[i].tmp].name,(int)(parts[i].x+.5),(int)(parts[i].y+.5));
				error_ui(vid_buf,0,errortext);
				return 1;
			}
		}
	}
	return 0;
}

pixel *prerender_save_OPS(void *save, int size, int *width, int *height)
{
	unsigned char * inputData = save, *bsonData = NULL, *partsData = NULL, *partsPosData = NULL, *wallData = NULL;
	int inputDataLen = size, bsonDataLen = 0, partsDataLen, partsPosDataLen, wallDataLen;
	int i, x, y, j, type, ctype;
	int blockX, blockY, blockW, blockH, fullX, fullY, fullW, fullH;
	int bsonInitialised = 0;
	pixel * vidBuf = NULL;
	bson b;
	bson_iterator iter;

	//Block sizes
	blockX = 0;
	blockY = 0;
	blockW = inputData[6];
	blockH = inputData[7];
	
	//Full size, normalised
	fullX = 0;
	fullY = 0;
	fullW = blockW*CELL;
	fullH = blockH*CELL;
	
	//
	*width = fullW;
	*height = fullH;
	
	//From newer version
	if(inputData[4] > SAVE_VERSION && inputData[4] < 200)
	{
		fprintf(stderr, "Save from newer version\n");
		//goto fail;
	}
		
	//Incompatible cell size
	if(inputData[5] > CELL)
	{
		fprintf(stderr, "Cell size mismatch\n");
		goto fail;
	}
		
	//Too large/off screen
	if(blockX+blockW > XRES/CELL || blockY+blockH > YRES/CELL)
	{
		fprintf(stderr, "Save too large\n");
		goto fail;
	}
	
	bsonDataLen = ((unsigned)inputData[8]);
	bsonDataLen |= ((unsigned)inputData[9]) << 8;
	bsonDataLen |= ((unsigned)inputData[10]) << 16;
	bsonDataLen |= ((unsigned)inputData[11]) << 24;
	
	bsonData = malloc(bsonDataLen+1);
	if(!bsonData)
	{
		fprintf(stderr, "Internal error while parsing save: could not allocate buffer\n");
		goto fail;
	}
	//Make sure bsonData is null terminated, since all string functions need null terminated strings
	//(bson_iterator_key returns a pointer into bsonData, which is then used with strcmp)
	bsonData[bsonDataLen] = 0;
	
	if (BZ2_bzBuffToBuffDecompress(bsonData, &bsonDataLen, inputData+12, inputDataLen-12, 0, 0))
	{
		fprintf(stderr, "Unable to decompress\n");
		goto fail;
	}
	
	bson_init_data(&b, bsonData);
	bsonInitialised = 1;
	bson_iterator_init(&iter, &b);
	while(bson_iterator_next(&iter))
	{
		if(strcmp(bson_iterator_key(&iter), "parts")==0)
		{
			if(bson_iterator_type(&iter)==BSON_BINDATA && ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER && (partsDataLen = bson_iterator_bin_len(&iter)) > 0)
			{
				partsData = (unsigned char*)bson_iterator_bin_data(&iter);
			}
			else
			{
				fprintf(stderr, "Invalid datatype of particle data: %d[%d] %d[%d] %d[%d]\n", bson_iterator_type(&iter), bson_iterator_type(&iter)==BSON_BINDATA, (unsigned char)bson_iterator_bin_type(&iter), ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER, bson_iterator_bin_len(&iter), bson_iterator_bin_len(&iter)>0);
			}
		}
		if(strcmp(bson_iterator_key(&iter), "partsPos")==0)
		{
			if(bson_iterator_type(&iter)==BSON_BINDATA && ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER && (partsPosDataLen = bson_iterator_bin_len(&iter)) > 0)
			{
				partsPosData = (unsigned char*)bson_iterator_bin_data(&iter);
			}
			else
			{
				fprintf(stderr, "Invalid datatype of particle position data: %d[%d] %d[%d] %d[%d]\n", bson_iterator_type(&iter), bson_iterator_type(&iter)==BSON_BINDATA, (unsigned char)bson_iterator_bin_type(&iter), ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER, bson_iterator_bin_len(&iter), bson_iterator_bin_len(&iter)>0);
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "wallMap")==0)
		{
			if(bson_iterator_type(&iter)==BSON_BINDATA && ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER && (wallDataLen = bson_iterator_bin_len(&iter)) > 0)
			{
				wallData = (unsigned char*)bson_iterator_bin_data(&iter);
			}
			else
			{
				fprintf(stderr, "Invalid datatype of wall data: %d[%d] %d[%d] %d[%d]\n", bson_iterator_type(&iter), bson_iterator_type(&iter)==BSON_BINDATA, (unsigned char)bson_iterator_bin_type(&iter), ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER, bson_iterator_bin_len(&iter), bson_iterator_bin_len(&iter)>0);
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "compatible_with")==0)
		{
			if(bson_iterator_type(&iter)==BSON_INT)
			{
				if (bson_iterator_int(&iter) > MOD_SAVE_VERSION)
				{
					fprintf(stderr, "Save is not compatible\n");
					goto fail;
				}
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
	}
	
	vidBuf = calloc(fullW*fullH, PIXELSIZE);
	
	//Read wall and fan data
	if(wallData)
	{
		if(blockW * blockH > wallDataLen)
		{
			fprintf(stderr, "Not enough wall data\n");
			goto fail;
		}
		for(x = 0; x < blockW; x++)
		{
			for(y = 0; y < blockH; y++)
			{
				if(wallData[y*blockW+x])
				{
					for(i = 0; i < CELL; i++)
					{
						for(j = 0; j < CELL; j++)
						{
							vidBuf[(fullY+i+(y*CELL))*fullW+(fullX+j+(x*CELL))] = PIXPACK(0xCCCCCC);
						}
					}
				};
			}
		}
	}
	
	//Read particle data
	if(partsData && partsPosData)
	{
		int fieldDescriptor;
		int posCount, posTotal, partsPosDataIndex = 0;
		int saved_x, saved_y;
		if(fullW * fullH * 3 > partsPosDataLen)
		{
			fprintf(stderr, "Not enough particle position data\n");
			goto fail;
		}
		i = 0;
		for (saved_y=0; saved_y<fullH; saved_y++)
		{
			for (saved_x=0; saved_x<fullW; saved_x++)
			{
				//Read total number of particles at this position
				posTotal = 0;
				posTotal |= partsPosData[partsPosDataIndex++]<<16;
				posTotal |= partsPosData[partsPosDataIndex++]<<8;
				posTotal |= partsPosData[partsPosDataIndex++];
				//Put the next posTotal particles at this position
				for (posCount=0; posCount<posTotal; posCount++)
				{
					//i+3 because we have 4 bytes of required fields (type (1), descriptor (2), temp (1))
					if (i+3 >= partsDataLen)
						goto fail;
					x = saved_x + fullX;
					y = saved_y + fullY;
					fieldDescriptor = partsData[i+1];
					fieldDescriptor |= partsData[i+2] << 8;
					if(x >= XRES || x < 0 || y >= YRES || y < 0)
					{
						fprintf(stderr, "Out of range [%d]: %d %d, [%d, %d], [%d, %d]\n", i, x, y, (unsigned)partsData[i+1], (unsigned)partsData[i+2], (unsigned)partsData[i+3], (unsigned)partsData[i+4]);
						goto fail;
					}
					type = partsData[i];
					if(type >= PT_NUM)
						type = PT_DMND;	//Replace all invalid elements with diamond
					
					//Draw type
					if (type==PT_STKM || type==PT_STKM2 || type==PT_FIGH)
					{
						pixel lc, hc=PIXRGB(255, 224, 178);
						if (type==PT_STKM || type==PT_FIGH) lc = PIXRGB(255, 255, 255);
						else lc = PIXRGB(100, 100, 255);
						//only need to check upper bound of y coord - lower bounds and x<w are checked in draw_line
						if (type==PT_STKM || type==PT_STKM2)
						{
							draw_line(vidBuf, x-2, y-2, x+2, y-2, PIXR(hc), PIXG(hc), PIXB(hc), *width);
							if (y+2<*height)
							{
								draw_line(vidBuf, x-2, y+2, x+2, y+2, PIXR(hc), PIXG(hc), PIXB(hc), *width);
								draw_line(vidBuf, x-2, y-2, x-2, y+2, PIXR(hc), PIXG(hc), PIXB(hc), *width);
								draw_line(vidBuf, x+2, y-2, x+2, y+2, PIXR(hc), PIXG(hc), PIXB(hc), *width);
							}
						}
						else if (y+2<*height)
						{
							draw_line(vidBuf, x-2, y, x, y-2, PIXR(hc), PIXG(hc), PIXB(hc), *width);
							draw_line(vidBuf, x-2, y, x, y+2, PIXR(hc), PIXG(hc), PIXB(hc), *width);
							draw_line(vidBuf, x, y-2, x+2, y, PIXR(hc), PIXG(hc), PIXB(hc), *width);
							draw_line(vidBuf, x, y+2, x+2, y, PIXR(hc), PIXG(hc), PIXB(hc), *width);
						}
						if (y+6<*height)
						{
							draw_line(vidBuf, x, y+3, x-1, y+6, PIXR(lc), PIXG(lc), PIXB(lc), *width);
							draw_line(vidBuf, x, y+3, x+1, y+6, PIXR(lc), PIXG(lc), PIXB(lc), *width);
						}
						if (y+12<*height)
						{
							draw_line(vidBuf, x-1, y+6, x-3, y+12, PIXR(lc), PIXG(lc), PIXB(lc), *width);
							draw_line(vidBuf, x+1, y+6, x+3, y+12, PIXR(lc), PIXG(lc), PIXB(lc), *width);
						}
					}
					else
						vidBuf[(fullY+y)*fullW+(fullX+x)] = ptypes[type].pcolors;
					i+=3; //Skip Type and Descriptor
					
					//Skip temp
					if(fieldDescriptor & 0x01)
					{
						i+=2;
					}
					else
					{
						i++;
					}
					
					//Skip life
					if(fieldDescriptor & 0x02)
					{
						if(i++ >= partsDataLen) goto fail;
						if(fieldDescriptor & 0x04)
						{
							if(i++ >= partsDataLen) goto fail;
						}
					}
					
					//Skip tmp
					if(fieldDescriptor & 0x08)
					{
						if(i++ >= partsDataLen) goto fail;
						if(fieldDescriptor & 0x10)
						{
							if(i++ >= partsDataLen) goto fail;
						}
					}
					
					//Skip ctype
					if(fieldDescriptor & 0x20)
					{
						if(i >= partsDataLen) goto fail;
						ctype = partsData[i++];
						if(fieldDescriptor & 0x200)
						{
							if(i+2 >= partsDataLen) goto fail;
							ctype |= (((unsigned)partsData[i++]) << 24);
							ctype |= (((unsigned)partsData[i++]) << 16);
							ctype |= (((unsigned)partsData[i++]) << 8);
						}
					}
					
					//Read dcolour
					if(fieldDescriptor & 0x40)
					{
						unsigned char r, g, b;
						if(i+3 >= partsDataLen) goto fail;
						i++;//Skip alpha
						r = partsData[i++];
						g = partsData[i++];
						b = partsData[i++];
						vidBuf[(fullY+y)*fullW+(fullX+x)] = PIXRGB(r, g, b);
					}
					
					//Skip vx
					if(fieldDescriptor & 0x80)
					{
						if(i++ >= partsDataLen) goto fail;
					}
					
					//Skip vy
					if(fieldDescriptor & 0x100)
					{
						if(i++ >= partsDataLen) goto fail;
					}

					//Skip tmp2
					if(fieldDescriptor & 0x400)
					{
						if(i++ >= partsDataLen) goto fail;
						if (type == PT_MOVS)
							if(i++ >= partsDataLen) goto fail;
					}

					//Skip animations
					if ((type == PT_ANIM || type == PT_INDI) && ctype)
					{
						int k;
						if (type == PT_ANIM)
							i += 4*ctype+4;
						else
							i += 1028;
						if(i > partsDataLen) goto fail;
					}
					if (type == PT_MOVS && !(fieldDescriptor & 0x08) && !(fieldDescriptor & 0x400))
						if(i++ >= partsDataLen) goto fail;
				}
			}
		}
	}
	goto fin;
fail:
	if(vidBuf)
	{
		free(vidBuf);
		vidBuf = NULL;
	}
fin:
	//Don't call bson_destroy if bson_init wasn't called, or an uninitialised pointer (b.data) will be freed and the game will crash
	if (bsonInitialised)
		bson_destroy(&b);
	return vidBuf;
}

void *build_save_OPS(int *size, int orig_x0, int orig_y0, int orig_w, int orig_h, unsigned char bmap[YRES/CELL][XRES/CELL], float vx[YRES/CELL][XRES/CELL], float vy[YRES/CELL][XRES/CELL], float pv[YRES/CELL][XRES/CELL], float fvx[YRES/CELL][XRES/CELL], float fvy[YRES/CELL][XRES/CELL], sign signs[MAXSIGNS], void* o_partsptr)
{
	particle *partsptr = o_partsptr;
	unsigned char *partsData = NULL, *partsPosData = NULL, *fanData = NULL, *wallData = NULL, *pressData = NULL, *finalData = NULL, *outputData = NULL;
	unsigned *partsPosLink = NULL, *partsPosFirstMap = NULL, *partsPosCount = NULL, *partsPosLastMap = NULL;
	int partsDataLen, partsPosDataLen, fanDataLen, wallDataLen, pressDataLen, finalDataLen, outputDataLen;
	int blockX, blockY, blockW, blockH, fullX, fullY, fullW, fullH;
	int x, y, i, wallDataFound = 0;
	int posCount, signsCount;
	bson b;

	//Get coords in blocks
	blockX = orig_x0/CELL;
	blockY = orig_y0/CELL;

	//Snap full coords to block size
	fullX = blockX*CELL;
	fullY = blockY*CELL;

	//Original size + offset of original corner from snapped corner, rounded up by adding CELL-1
	blockW = (orig_w+orig_x0-fullX+CELL-1)/CELL;
	blockH = (orig_h+orig_y0-fullY+CELL-1)/CELL;
	fullW = blockW*CELL;
	fullH = blockH*CELL;
	
	//Copy fan and wall data
	wallData = malloc(blockW*blockH);
	wallDataLen = blockW*blockH;
	fanData = malloc((blockW*blockH)*2);
	fanDataLen = 0;
	pressData = (unsigned char*)malloc((blockW*blockH)*2);
	pressDataLen = 0;
	for(x = blockX; x < blockX+blockW; x++)
	{
		for(y = blockY; y < blockY+blockH; y++)
		{
			wallData[(y-blockY)*blockW+(x-blockX)] = bmap[y][x];
			if (0 && save_as == 3)
			{
				float pres = (max(-255,min(255,pv[y][x])))+256;
				pressData[pressDataLen++] = (unsigned char)((int)(pres*128)&0xFF);
				pressData[pressDataLen++] = (unsigned char)((int)(pres*128)>>8);
			}
			if(bmap[y][x] && !wallDataFound)
				wallDataFound = 1;
			if(bmap[y][x]==WL_FAN || bmap[y][x]==4)
			{
				i = (int)(fvx[y][x]*64.0f+127.5f);
				if (i<0) i=0;
				if (i>255) i=255;
				fanData[fanDataLen++] = i;
				i = (int)(fvy[y][x]*64.0f+127.5f);
				if (i<0) i=0;
				if (i>255) i=255;
				fanData[fanDataLen++] = i;
			}
		}
	}
	if(!fanDataLen)
	{
		free(fanData);
		fanData = NULL;
	}
	if(!wallDataFound)
	{
		free(wallData);
		wallData = NULL;
	}
	
	//Index positions of all particles, using linked lists
	//partsPosFirstMap is pmap for the first particle in each position
	//partsPosLastMap is pmap for the last particle in each position
	//partsPosCount is the number of particles in each position
	//partsPosLink contains, for each particle, (i<<8)|1 of the next particle in the same position
	partsPosFirstMap = calloc(fullW*fullH, sizeof(unsigned));
	partsPosLastMap = calloc(fullW*fullH, sizeof(unsigned));
	partsPosCount = calloc(fullW*fullH, sizeof(unsigned));
	partsPosLink = calloc(NPART, sizeof(unsigned));
	for(i = 0; i < NPART; i++)
	{
		if(partsptr[i].type)
		{
			x = (int)(partsptr[i].x+0.5f);
			y = (int)(partsptr[i].y+0.5f);
			if (x>=orig_x0 && x<orig_x0+orig_w && y>=orig_y0 && y<orig_y0+orig_h)
			{
				//Coordinates relative to top left corner of saved area
				x -= fullX;
				y -= fullY;
				if (!partsPosFirstMap[y*fullW + x])
				{
					//First entry in list
					partsPosFirstMap[y*fullW + x] = (i<<8)|1;
					partsPosLastMap[y*fullW + x] = (i<<8)|1;
				}
				else
				{
					//Add to end of list
					partsPosLink[partsPosLastMap[y*fullW + x]>>8] = (i<<8)|1;//link to current end of list
					partsPosLastMap[y*fullW + x] = (i<<8)|1;//set as new end of list
				}
				partsPosCount[y*fullW + x]++;
			}
		}
	}

	//Store number of particles in each position
	partsPosData = malloc(fullW*fullH*3);
	partsPosDataLen = 0;
	for (y=0;y<fullH;y++)
	{
		for (x=0;x<fullW;x++)
		{
			posCount = partsPosCount[y*fullW + x];
			partsPosData[partsPosDataLen++] = (posCount&0x00FF0000)>>16;
			partsPosData[partsPosDataLen++] = (posCount&0x0000FF00)>>8;
			partsPosData[partsPosDataLen++] = (posCount&0x000000FF);
		}
	}

	i = pmap[4][4]>>8;
	if (parts[i].type == PT_INDI && parts[i].animations)
		parts[i].animations[1] = 0; //Save lua code as not being run yet, so it will run when the save is opened
	//Copy parts data
	/* Field descriptor format:
	|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|		0		|
																					|		tmp2	|	ctype[2]	|		vy		|		vx		|	dcololour	|	ctype[1]	|		tmp[2]	|		tmp[1]	|		life[2]	|		life[1]	|	temp dbl len|
	life[2] means a second byte (for a 16 bit field) if life[1] is present
	*/
	partsData = malloc(NPART * (sizeof(particle)+1));
	partsDataLen = 0;
	for (y=0;y<fullH;y++)
	{
		for (x=0;x<fullW;x++)
		{
			//Find the first particle in this position
			i = partsPosFirstMap[y*fullW + x];

			//Loop while there is a pmap entry
			while (i)
			{
				unsigned short fieldDesc = 0;
				int fieldDescLoc = 0, tempTemp, vTemp;
				
				//Turn pmap entry into a partsptr index
				i = i>>8;

				//Type (required)
				partsData[partsDataLen++] = partsptr[i].type;
				
				//Location of the field descriptor
				fieldDescLoc = partsDataLen++;
				partsDataLen++;
				
				//Extra Temperature (2nd byte optional, 1st required), 1 to 2 bytes
				//Store temperature as an offset of 21C(294.15K) or go into a 16byte int and store the whole thing
				if(fabs(partsptr[i].temp-294.15f)<127)
				{
					tempTemp = (int)(partsptr[i].temp-294.15f);
					partsData[partsDataLen++] = tempTemp;
				}
				else
				{
					fieldDesc |= 1;
					tempTemp = (int)partsptr[i].temp;
					partsData[partsDataLen++] = tempTemp;
					partsData[partsDataLen++] = tempTemp >> 8;
				}
				
				//Life (optional), 1 to 2 bytes
				if(partsptr[i].life)
				{
					fieldDesc |= 1 << 1;
					partsData[partsDataLen++] = partsptr[i].life;
					if(partsptr[i].life > 255)
					{
						fieldDesc |= 1 << 2;
						partsData[partsDataLen++] = partsptr[i].life >> 8;
					}
				}
				
				//Tmp (optional), 1 to 2 bytes
				if(partsptr[i].tmp)
				{
					fieldDesc |= 1 << 3;
					partsData[partsDataLen++] = partsptr[i].tmp;
					if(partsptr[i].tmp > 255 || partsptr[i].type == PT_MOVS)
					{
						fieldDesc |= 1 << 4;
						partsData[partsDataLen++] = partsptr[i].tmp >> 8;
					}
				}
				
				//Ctype (optional), 1 or 4 bytes
				if(partsptr[i].ctype)
				{
					fieldDesc |= 1 << 5;
					partsData[partsDataLen++] = partsptr[i].ctype;
					if(partsptr[i].ctype > 255)
					{
						fieldDesc |= 1 << 9;
						partsData[partsDataLen++] = (partsptr[i].ctype&0xFF000000)>>24;
						partsData[partsDataLen++] = (partsptr[i].ctype&0x00FF0000)>>16;
						partsData[partsDataLen++] = (partsptr[i].ctype&0x0000FF00)>>8;
					}
				}
				
				//Dcolour (optional), 4 bytes
				if(partsptr[i].dcolour && (partsptr[i].dcolour & 0xFF000000))
				{
					fieldDesc |= 1 << 6;
					partsData[partsDataLen++] = (partsptr[i].dcolour&0xFF000000)>>24;
					partsData[partsDataLen++] = (partsptr[i].dcolour&0x00FF0000)>>16;
					partsData[partsDataLen++] = (partsptr[i].dcolour&0x0000FF00)>>8;
					partsData[partsDataLen++] = (partsptr[i].dcolour&0x000000FF);
				}
				
				//VX (optional), 1 byte
				if(fabs(partsptr[i].vx) > 0.001f)
				{
					fieldDesc |= 1 << 7;
					vTemp = (int)(partsptr[i].vx*16.0f+127.5f);
					if (vTemp<0) vTemp=0;
					if (vTemp>255) vTemp=255;
					partsData[partsDataLen++] = vTemp;
				}
				
				//VY (optional), 1 byte
				if(fabs(partsptr[i].vy) > 0.001f)
				{
					fieldDesc |= 1 << 8;
					vTemp = (int)(partsptr[i].vy*16.0f+127.5f);
					if (vTemp<0) vTemp=0;
					if (vTemp>255) vTemp=255;
					partsData[partsDataLen++] = vTemp;
				}

				//Tmp2 (optional), 1 byte
				if(partsptr[i].tmp2)
				{
					fieldDesc |= 1 << 10;
					partsData[partsDataLen++] = partsptr[i].tmp2;
					if (partsptr[i].type == PT_MOVS)
						partsData[partsDataLen++] = partsptr[i].tmp2 >> 8;
				}

				if (save_as == 3)
				{
					if ((partsptr[i].type == PT_ANIM || partsptr[i].type == PT_INDI) &&  partsptr[i].ctype)
					{
						int j, max = partsptr[i].ctype;
						if (partsptr[i].type == PT_INDI)
							max = 256;
						for (j = 0; j <=max;j++)
						{
							partsData[partsDataLen++] = (partsptr[i].animations[j]&0xFF000000)>>24;
							partsData[partsDataLen++] = (partsptr[i].animations[j]&0x00FF0000)>>16;
							partsData[partsDataLen++] = (partsptr[i].animations[j]&0x0000FF00)>>8;
							partsData[partsDataLen++] = (partsptr[i].animations[j]&0x000000FF);
						}
					}
					if (partsptr[i].type == PT_MOVS && !partsptr[i].tmp && !partsptr[i].tmp2 && i == msindex[partsptr[i].life]-1)
					{
						partsData[partsDataLen++] = (int)((msrotation[partsptr[i].life] + 6.283185307179586476925286766559)*20);
					}
				}
				
				//Write the field descriptor;
				partsData[fieldDescLoc] = fieldDesc;
				partsData[fieldDescLoc+1] = fieldDesc>>8;

				//Get the pmap entry for the next particle in the same position
				i = partsPosLink[i];
			}
		}
	}
	if(!partsDataLen)
	{
		free(partsData);
		partsData = NULL;
	}
	
	bson_init(&b);
	bson_append_bool(&b, "waterEEnabled", water_equal_test);
	bson_append_bool(&b, "legacyEnable", legacy_enable);
	bson_append_bool(&b, "gravityEnable", ngrav_enable);
	bson_append_bool(&b, "paused", sys_pause);
	bson_append_int(&b, "gravityMode", gravityMode);
	bson_append_int(&b, "airMode", airMode);
	bson_append_int(&b, "numballs", numballs);
	bson_append_bool(&b, "msrotation", ms_rotation);
	bson_append_bool(&b, "decorations_enaable", decorations_enable);
	bson_append_bool(&b, "hud_enable", hud_enable);
	bson_append_bool(&b, "aheat_enable", aheat_enable);
	bson_append_int(&b, "render_mode", render_mode);
	bson_append_int(&b, "display_mode", display_mode);
	bson_append_int(&b, "color_mode", colour_mode);
	bson_append_int(&b, "Jacob1's_Mod", MOD_SAVE_VERSION);
	bson_append_int(&b, "compatible_with", 8);
	
	//bson_append_int(&b, "leftSelectedElement", sl);
	//bson_append_int(&b, "rightSelectedElement", sr);
	bson_append_int(&b, "activeMenu", active_menu);
	if(partsData)
		bson_append_binary(&b, "parts", BSON_BIN_USER, partsData, partsDataLen);
	if(partsPosData)
		bson_append_binary(&b, "partsPos", BSON_BIN_USER, partsPosData, partsPosDataLen);
	if(wallData)
		bson_append_binary(&b, "wallMap", BSON_BIN_USER, wallData, wallDataLen);
	if(fanData)
		bson_append_binary(&b, "fanMap", BSON_BIN_USER, fanData, fanDataLen);
	if(pressData)
		bson_append_binary(&b, "pressMap", BSON_BIN_USER, (const char*)pressData, pressDataLen);
	signsCount = 0;
	for(i = 0; i < MAXSIGNS; i++)
	{
		if(signs[i].text[0] && signs[i].x>=fullX && signs[i].x<=fullX+fullW && signs[i].y>=fullY && signs[i].y<=fullY+fullH)
		{
			signsCount++;
		}
	}
	if(signsCount)
	{
		bson_append_start_array(&b, "signs");
		for(i = 0; i < MAXSIGNS; i++)
		{
			if(signs[i].text[0] && signs[i].x>=fullX && signs[i].x<=fullX+fullW && signs[i].y>=fullY && signs[i].y<=fullY+fullH)
			{
				bson_append_start_object(&b, "sign");
				bson_append_string(&b, "text", signs[i].text);
				bson_append_int(&b, "justification", signs[i].ju);
				bson_append_int(&b, "x", signs[i].x-fullX);
				bson_append_int(&b, "y", signs[i].y-fullY);
				bson_append_finish_object(&b);
			}
		}
	}
	bson_append_finish_array(&b);
	bson_finish(&b);
	bson_print(&b);
	
	finalData = (unsigned char*)bson_data(&b);
	finalDataLen = bson_size(&b);
	outputDataLen = finalDataLen*2+12;
	outputData = (unsigned char*)malloc(outputDataLen);

	outputData[0] = 'O';
	outputData[1] = 'P';
	outputData[2] = 'S';
	outputData[3] = '1';
	outputData[4] = SAVE_VERSION;
	outputData[5] = CELL;
	outputData[6] = blockW;
	outputData[7] = blockH;
	outputData[8] = finalDataLen;
	outputData[9] = finalDataLen >> 8;
	outputData[10] = finalDataLen >> 16;
	outputData[11] = finalDataLen >> 24;
	
	if (BZ2_bzBuffToBuffCompress(outputData+12, &outputDataLen, finalData, bson_size(&b), 9, 0, 0) != BZ_OK)
	{
		puts("Save Error\n");
		free(outputData);
		*size = 0;
		outputData = NULL;
		goto fin;
	}
	
	printf("compressed data: %d\n", outputDataLen);
	*size = outputDataLen + 12;
	
fin:
	bson_destroy(&b);
	if(partsData)
		free(partsData);
	if(wallData)
		free(wallData);
	if(fanData)
		free(fanData);
	
	return outputData;
}

int parse_save_OPS(void *save, int size, int replace, int x0, int y0, unsigned char bmap[YRES/CELL][XRES/CELL], float vx[YRES/CELL][XRES/CELL], float vy[YRES/CELL][XRES/CELL], float pv[YRES/CELL][XRES/CELL], float fvx[YRES/CELL][XRES/CELL], float fvy[YRES/CELL][XRES/CELL], sign signs[MAXSIGNS], void* o_partsptr, unsigned pmap[YRES][XRES])
{
	particle *partsptr = o_partsptr;
	unsigned char * inputData = save, *bsonData = NULL, *partsData = NULL, *partsPosData = NULL, *fanData = NULL, *wallData = NULL, *pressData = NULL;
	int inputDataLen = size, bsonDataLen = 0, partsDataLen, partsPosDataLen, fanDataLen, wallDataLen, pressDataLen;
	int i, freeIndicesCount, x, y, returnCode = 0, j, oldnumballs = numballs;
	int *freeIndices = NULL;
	int blockX, blockY, blockW, blockH, fullX, fullY, fullW, fullH;
	bson b;
	bson_iterator iter;

	//Block sizes
	blockX = x0/CELL;
	blockY = y0/CELL;
	blockW = inputData[6];
	blockH = inputData[7];
	
	//Full size, normalised
	fullX = blockX*CELL;
	fullY = blockY*CELL;
	fullW = blockW*CELL;
	fullH = blockH*CELL;
	
	//From newer version
	if(inputData[4] > SAVE_VERSION && inputData[4] > SAVE_VERSION < 200)
	{
		info_ui(vid_buf,"Save is from a newer version","Attempting to load it anyway, this may cause a crash");
	}
		
	//Incompatible cell size
	if(inputData[5] > CELL)
	{
		fprintf(stderr, "Cell size mismatch\n");
		return 1;
	}
		
	//Too large/off screen
	if(blockX+blockW > XRES/CELL || blockY+blockH > YRES/CELL)
	{
		fprintf(stderr, "Save too large\n");
		return 1;
	}
	
	bsonDataLen = ((unsigned)inputData[8]);
	bsonDataLen |= ((unsigned)inputData[9]) << 8;
	bsonDataLen |= ((unsigned)inputData[10]) << 16;
	bsonDataLen |= ((unsigned)inputData[11]) << 24;
	
	bsonData = malloc(bsonDataLen+1);
	if(!bsonData)
	{
		fprintf(stderr, "Internal error while parsing save: could not allocate buffer\n");
		return 3;
	}
	//Make sure bsonData is null terminated, since all string functions need null terminated strings
	//(bson_iterator_key returns a pointer into bsonData, which is then used with strcmp)
	bsonData[bsonDataLen] = 0;
	
	if (BZ2_bzBuffToBuffDecompress(bsonData, &bsonDataLen, inputData+12, inputDataLen-12, 0, 0))
	{
		fprintf(stderr, "Unable to decompress\n");
		return 1;
	}
	
	if(replace)
	{
		//Remove everything
		clear_sim();
		memset(msindex, 0, sizeof(msindex));
		memset(msnum, 0, sizeof(msnum));
		memset(msvx, 0, sizeof(msvx));
		memset(msvy, 0, sizeof(msvy));
		memset(msrotation, 0, sizeof(msrotation));
		oldnumballs = 0;
	}
	
	bson_init_data(&b, bsonData);
	bson_iterator_init(&iter, &b);
	while(bson_iterator_next(&iter))
	{
		if(strcmp(bson_iterator_key(&iter), "signs")==0)
		{
			if(bson_iterator_type(&iter)==BSON_ARRAY)
			{
				bson_iterator subiter;
				bson_iterator_subiterator(&iter, &subiter);
				while(bson_iterator_next(&subiter))
				{
					if(strcmp(bson_iterator_key(&subiter), "sign")==0)
					{
						if(bson_iterator_type(&subiter)==BSON_OBJECT)
						{
							bson_iterator signiter;
							bson_iterator_subiterator(&subiter, &signiter);
							//Find a free sign ID
							for (i = 0; i < MAXSIGNS; i++)
								if (!signs[i].text[0])
									break;
							//Stop reading signs if we have no free spaces
							if(i >= MAXSIGNS)
								break;
							while(bson_iterator_next(&signiter))
							{
								if(strcmp(bson_iterator_key(&signiter), "text")==0 && bson_iterator_type(&signiter)==BSON_STRING)
								{
									strcpy(signs[i].text, bson_iterator_string(&signiter));
									clean_text(signs[i].text, 158-14);
								}
								else if(strcmp(bson_iterator_key(&signiter), "justification")==0 && bson_iterator_type(&signiter)==BSON_INT)
								{
									signs[i].ju = bson_iterator_int(&signiter);
								}
								else if(strcmp(bson_iterator_key(&signiter), "x")==0 && bson_iterator_type(&signiter)==BSON_INT)
								{
									signs[i].x = bson_iterator_int(&signiter)+fullX;
								}
								else if(strcmp(bson_iterator_key(&signiter), "y")==0 && bson_iterator_type(&signiter)==BSON_INT)
								{
									signs[i].y = bson_iterator_int(&signiter)+fullY;
								}
								else
								{
									fprintf(stderr, "Unknown sign property %s\n", bson_iterator_key(&signiter));
								}
							}
						}
						else
						{
							fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&subiter));
						}
					}
				}
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "parts")==0)
		{
			if(bson_iterator_type(&iter)==BSON_BINDATA && ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER && (partsDataLen = bson_iterator_bin_len(&iter)) > 0)
			{
				partsData = (unsigned char*)bson_iterator_bin_data(&iter);
			}
			else
			{
				fprintf(stderr, "Invalid datatype of particle data: %d[%d] %d[%d] %d[%d]\n", bson_iterator_type(&iter), bson_iterator_type(&iter)==BSON_BINDATA, (unsigned char)bson_iterator_bin_type(&iter), ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER, bson_iterator_bin_len(&iter), bson_iterator_bin_len(&iter)>0);
			}
		}
		if(strcmp(bson_iterator_key(&iter), "partsPos")==0)
		{
			if(bson_iterator_type(&iter)==BSON_BINDATA && ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER && (partsPosDataLen = bson_iterator_bin_len(&iter)) > 0)
			{
				partsPosData = (unsigned char*)bson_iterator_bin_data(&iter);
			}
			else
			{
				fprintf(stderr, "Invalid datatype of particle position data: %d[%d] %d[%d] %d[%d]\n", bson_iterator_type(&iter), bson_iterator_type(&iter)==BSON_BINDATA, (unsigned char)bson_iterator_bin_type(&iter), ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER, bson_iterator_bin_len(&iter), bson_iterator_bin_len(&iter)>0);
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "wallMap")==0)
		{
			if(bson_iterator_type(&iter)==BSON_BINDATA && ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER && (wallDataLen = bson_iterator_bin_len(&iter)) > 0)
			{
				wallData = (unsigned char*)bson_iterator_bin_data(&iter);
			}
			else
			{
				fprintf(stderr, "Invalid datatype of wall data: %d[%d] %d[%d] %d[%d]\n", bson_iterator_type(&iter), bson_iterator_type(&iter)==BSON_BINDATA, (unsigned char)bson_iterator_bin_type(&iter), ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER, bson_iterator_bin_len(&iter), bson_iterator_bin_len(&iter)>0);
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "pressMap")==0 && replace)
		{
			if(bson_iterator_type(&iter)==BSON_BINDATA && ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER && (pressDataLen = bson_iterator_bin_len(&iter)) > 0)
			{
				pressData = (unsigned char*)bson_iterator_bin_data(&iter);
			}
			else
			{
				fprintf(stderr, "Invalid datatype of pressure data: %d[%d] %d[%d] %d[%d]\n", bson_iterator_type(&iter), bson_iterator_type(&iter)==BSON_BINDATA, (unsigned char)bson_iterator_bin_type(&iter), ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER, bson_iterator_bin_len(&iter), bson_iterator_bin_len(&iter)>0);
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "fanMap")==0)
		{
			if(bson_iterator_type(&iter)==BSON_BINDATA && ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER && (fanDataLen = bson_iterator_bin_len(&iter)) > 0)
			{
				fanData = (unsigned char*)bson_iterator_bin_data(&iter);
			}
			else
			{
				fprintf(stderr, "Invalid datatype of fan data: %d[%d] %d[%d] %d[%d]\n", bson_iterator_type(&iter), bson_iterator_type(&iter)==BSON_BINDATA, (unsigned char)bson_iterator_bin_type(&iter), ((unsigned char)bson_iterator_bin_type(&iter))==BSON_BIN_USER, bson_iterator_bin_len(&iter), bson_iterator_bin_len(&iter)>0);
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "legacyEnable")==0 && replace)
		{
			if(bson_iterator_type(&iter)==BSON_BOOL)
			{
				legacy_enable = ((int)bson_iterator_bool(&iter))?1:0;
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "gravityEnable")==0 && replace)
		{
			if(bson_iterator_type(&iter)==BSON_BOOL)
			{
				int tempGrav = ngrav_enable;
				tempGrav = ((int)bson_iterator_bool(&iter))?1:0;
#ifndef RENDERER
				//Change the gravity state
				if(ngrav_enable != tempGrav)
				{
					if(tempGrav)
						start_grav_async();
					else
						stop_grav_async();
				}
#endif
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "waterEEnabled")==0 && replace)
		{
			if(bson_iterator_type(&iter)==BSON_BOOL)
			{
				water_equal_test = ((int)bson_iterator_bool(&iter))?1:0;
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "paused")==0 && !sys_pause)
		{
			if(bson_iterator_type(&iter)==BSON_BOOL)
			{
				sys_pause = ((int)bson_iterator_bool(&iter))?1:0;
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "gravityMode")==0 && replace)
		{
			if(bson_iterator_type(&iter)==BSON_INT)
			{
				gravityMode = bson_iterator_int(&iter);
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "airMode")==0 && replace)
		{
			if(bson_iterator_type(&iter)==BSON_INT)
			{
				airMode = bson_iterator_int(&iter);
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		/*else if((strcmp(bson_iterator_key(&iter), "leftSelectedElement")==0 || strcmp(bson_iterator_key(&iter), "rightSelectedElement")) && replace)
		{
			if(bson_iterator_type(&iter)==BSON_INT && bson_iterator_int(&iter) > 0 && bson_iterator_int(&iter) < PT_NUM)
			{
				if(bson_iterator_key(&iter)[0] == 'l')
				{
					sl = bson_iterator_int(&iter);
				}
				else
				{
					sr = bson_iterator_int(&iter);
				}
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}*/
		else if(strcmp(bson_iterator_key(&iter), "activeMenu")==0 && replace)
		{
			if(bson_iterator_type(&iter)==BSON_INT && bson_iterator_int(&iter) > 0 && bson_iterator_int(&iter) < SC_TOTAL && msections[bson_iterator_int(&iter)].doshow)
			{
				active_menu = bson_iterator_int(&iter);
			}
			else
			{
				fprintf(stderr, "Wrong value for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "numballs")==0)
		{
			if(bson_iterator_type(&iter)==BSON_INT)
			{
				numballs += bson_iterator_int(&iter);
				if (numballs >= 256)
					numballs = 255;
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "msrotation")==0 && replace)
		{
			if(bson_iterator_type(&iter)==BSON_BOOL)
			{
				ms_rotation = ((int)bson_iterator_bool(&iter))?1:0;
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "decorations_enable")==0 && replace)
		{
			if(bson_iterator_type(&iter)==BSON_BOOL)
			{
				decorations_enable = ((int)bson_iterator_bool(&iter))?1:0;
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "hud_enable")==0 && replace)
		{
			if(bson_iterator_type(&iter)==BSON_BOOL)
			{
				hud_enable = ((int)bson_iterator_bool(&iter))?1:0;
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "aheat_enable")==0 && replace)
		{
			if(bson_iterator_type(&iter)==BSON_BOOL)
			{
				aheat_enable = ((int)bson_iterator_bool(&iter))?1:0;
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "render_mode")==0 && replace)
		{
			if(bson_iterator_type(&iter)==BSON_INT)
			{
				render_mode = bson_iterator_int(&iter);
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "display_mode")==0 && replace)
		{
			if(bson_iterator_type(&iter)==BSON_INT)
			{
				display_mode = bson_iterator_int(&iter);
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "color_mode")==0 && replace)
		{
			if(bson_iterator_type(&iter)==BSON_INT)
			{
				colour_mode = bson_iterator_int(&iter);
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
		else if(strcmp(bson_iterator_key(&iter), "compatible_with")==0)
		{
			if(bson_iterator_type(&iter)==BSON_INT)
			{
				if (bson_iterator_int(&iter) > MOD_SAVE_VERSION)
				{
					fprintf(stderr, "Save is not compatible\n");
					return 2;
				}
			}
			else
			{
				fprintf(stderr, "Wrong type for %s\n", bson_iterator_key(&iter));
			}
		}
	}
	
	//Read wall and fan data
	if(wallData)
	{
		j = 0;
		if(blockW * blockH > wallDataLen)
		{
			fprintf(stderr, "Not enough wall data\n");
			goto fail;
		}
		for(x = 0; x < blockW; x++)
		{
			for(y = 0; y < blockH; y++)
			{
				bmap[blockY+y][blockX+x] = wallData[y*blockW+x];
				if((bmap[blockY+y][blockX+x]==WL_FAN || bmap[blockY+y][blockX+x]==4) && fanData)
				{
					if(j+1 >= fanDataLen)
					{
						fprintf(stderr, "Not enough fan data\n");
					}
					fvx[blockY+y][blockX+x] = (fanData[j++]-127.0f)/64.0f;
					fvy[blockY+y][blockX+x] = (fanData[j++]-127.0f)/64.0f;
				}
			}
		}
	}
	
	//Read pressure data
	if(pressData)
	{
		j = 0;
		if(blockW * blockH > pressDataLen)
		{
			fprintf(stderr, "Not enough pressure data\n");
			goto fail;
		}
		/*pressData[(y-blockY)*blockW+(x-blockX)*2] = (unsigned char)((int)(max(0,pv[y][x]+256)*128)&0xFF);
		  pressData[(y-blockY)*blockW+(x-blockX)*2+1] = (unsigned char)((int)(max(0,pv[y][x]+256)*128)>>8);*/
		for(x = 0; x < blockW; x++)
		{
			for(y = 0; y < blockH; y++)
			{
				int i2;
				i = pressData[j++];
				i2 = pressData[j++];
				printf("%f\n",(i+(i2<<8)));
				printf("%f\n\n",((i+(i2<<8))/128.0f)-256);
				pv[blockY+y][blockX+x] = ((i+(i2<<8))/128.0f)-256;
			}
		}
	}

	//Read particle data
	if(partsData && partsPosData)
	{
		int newIndex = 0, fieldDescriptor, tempTemp;
		int posCount, posTotal, partsPosDataIndex = 0;
		int saved_x, saved_y;
		int freeIndicesIndex = 0;
		if(fullW * fullH * 3 > partsPosDataLen)
		{
			fprintf(stderr, "Not enough particle position data\n");
			goto fail;
		}
		parts_lastActiveIndex = NPART-1;
		freeIndicesCount = 0;
		freeIndices = calloc(sizeof(int), NPART);
		for (i = 0; i<NPART; i++)
		{
			//Ensure ALL parts (even photons) are in the pmap so we can overwrite, keep a track of indices we can use
			if (partsptr[i].type)
			{
				x = (int)(partsptr[i].x+0.5f);
				y = (int)(partsptr[i].y+0.5f);
				pmap[y][x] = (i<<8)|1;
			}
			else
				freeIndices[freeIndicesCount++] = i;
		}
		i = 0;
		for (saved_y=0; saved_y<fullH; saved_y++)
		{
			for (saved_x=0; saved_x<fullW; saved_x++)
			{
				//Read total number of particles at this position
				posTotal = 0;
				posTotal |= partsPosData[partsPosDataIndex++]<<16;
				posTotal |= partsPosData[partsPosDataIndex++]<<8;
				posTotal |= partsPosData[partsPosDataIndex++];
				//Put the next posTotal particles at this position
				for (posCount=0; posCount<posTotal; posCount++)
				{
					//i+3 because we have 4 bytes of required fields (type (1), descriptor (2), temp (1))
					if (i+3 >= partsDataLen)
						goto fail;
					x = saved_x + fullX;
					y = saved_y + fullY;
					fieldDescriptor = partsData[i+1];
					fieldDescriptor |= partsData[i+2] << 8;
					if(x >= XRES || x < 0 || y >= YRES || y < 0)
					{
						fprintf(stderr, "Out of range [%d]: %d %d, [%d, %d], [%d, %d]\n", i, x, y, (unsigned)partsData[i+1], (unsigned)partsData[i+2], (unsigned)partsData[i+3], (unsigned)partsData[i+4]);
						goto fail;
					}
					if(partsData[i] >= PT_NUM)
						partsData[i] = PT_DMND;	//Replace all invalid elements with diamond
					if(pmap[y][x])
					{
						//Replace existing particle or allocated block
						newIndex = pmap[y][x]>>8;
					}
					else if(freeIndicesIndex<freeIndicesCount)
					{
						//Create new particle
						newIndex = freeIndices[freeIndicesIndex++];
					}
					else
					{
						//Nowhere to put new particle, tpt is sad :(
						break;
					}
					if(newIndex < 0 || newIndex >= NPART)
						goto fail;
						
					//Clear the particle, ready for our new properties
					memset(&(partsptr[newIndex]), 0, sizeof(particle));
					
					//Required fields
					partsptr[newIndex].type = partsData[i];
					partsptr[newIndex].x = (float)x;
					partsptr[newIndex].y = (float)y;
					i+=3;
					
					//Read temp
					if(fieldDescriptor & 0x01)
					{
						//Full 16bit int
						tempTemp = partsData[i++];
						tempTemp |= (((unsigned)partsData[i++]) << 8);
						partsptr[newIndex].temp = (float)tempTemp;
					}
					else
					{
						//1 Byte room temp offset
						tempTemp = (char)partsData[i++];
						partsptr[newIndex].temp = tempTemp+294.15f;
					}
					
					//Read life
					if(fieldDescriptor & 0x02)
					{
						if(i >= partsDataLen) goto fail;
						partsptr[newIndex].life = partsData[i++];
						//Read 2nd byte
						if(fieldDescriptor & 0x04)
						{
							if(i >= partsDataLen) goto fail;
							partsptr[newIndex].life |= (((unsigned)partsData[i++]) << 8);
						}
					}
					
					//Read tmp
					if(fieldDescriptor & 0x08)
					{
						if(i >= partsDataLen) goto fail;
						partsptr[newIndex].tmp = partsData[i++];
						//Read 2nd byte
						if(fieldDescriptor & 0x10)
						{
							if(i >= partsDataLen) goto fail;
							partsptr[newIndex].tmp |= (((unsigned)partsData[i++]) << 8);
						}
					}
					
					//Read ctype
					if(fieldDescriptor & 0x20)
					{
						if(i >= partsDataLen) goto fail;
						partsptr[newIndex].ctype = partsData[i++];
						//Read additional bytes
						if(fieldDescriptor & 0x200)
						{
							if(i+2 >= partsDataLen) goto fail;
							partsptr[newIndex].ctype |= (((unsigned)partsData[i++]) << 24);
							partsptr[newIndex].ctype |= (((unsigned)partsData[i++]) << 16);
							partsptr[newIndex].ctype |= (((unsigned)partsData[i++]) << 8);
						}
					}
					
					//Read dcolour
					if(fieldDescriptor & 0x40)
					{
						if(i+3 >= partsDataLen) goto fail;
						partsptr[newIndex].dcolour = (((unsigned)partsData[i++]) << 24);
						partsptr[newIndex].dcolour |= (((unsigned)partsData[i++]) << 16);
						partsptr[newIndex].dcolour |= (((unsigned)partsData[i++]) << 8);
						partsptr[newIndex].dcolour |= ((unsigned)partsData[i++]);
					}
					
					//Read vx
					if(fieldDescriptor & 0x80)
					{
						if(i >= partsDataLen) goto fail;
						partsptr[newIndex].vx = (partsData[i++]-127.0f)/16.0f;
					}
					
					//Read vy
					if(fieldDescriptor & 0x100)
					{
						if(i >= partsDataLen) goto fail;
						partsptr[newIndex].vy = (partsData[i++]-127.0f)/16.0f;
					}

					//Read tmp2
					if(fieldDescriptor & 0x400)
					{
						if(i >= partsDataLen) goto fail;
						partsptr[newIndex].tmp2 = partsData[i++];
						if (partsptr[newIndex].type == PT_MOVS)
							partsptr[newIndex].tmp2 |= (((unsigned)partsData[i++]) << 8);
					}

					if ((partsptr[newIndex].type == PT_ANIM || partsptr[newIndex].type == PT_INDI) && partsptr[newIndex].ctype)
					{
						int k;
						if (partsptr[newIndex].type == PT_ANIM)
						{
							if (partsptr[newIndex].ctype > maxframes)
								maxframes = partsptr[newIndex].ctype;
							partsptr[newIndex].animations = (unsigned int*)calloc(maxframes,sizeof(unsigned int));
						}
						else
							partsptr[newIndex].animations = (unsigned int*)calloc(257,sizeof(unsigned int));
						if (i+4+4*partsptr[newIndex].ctype >= partsDataLen || partsptr[newIndex].animations == NULL)
							goto fail;
						memset(partsptr[newIndex].animations, 0, sizeof(partsptr[newIndex].animations));
						for (k = 0; k <= partsptr[newIndex].ctype; k++)
						{
							partsptr[newIndex].animations[k] = partsData[i++]<<24;
							partsptr[newIndex].animations[k] |= partsData[i++]<<16;
							partsptr[newIndex].animations[k] |= partsData[i++]<<8;
							partsptr[newIndex].animations[k] |= partsData[i++];
						}
					}

					// no more particle properties to load, so we can change type here without messing up loading
					if ((player.spwn == 1 && partsptr[newIndex].type==PT_STKM) || (player2.spwn == 1 && partsptr[newIndex].type==PT_STKM2))
					{
						partsptr[newIndex].type = PT_NONE;
					}
					else if (partsptr[newIndex].type == PT_STKM)
					{
						STKM_init_legs(&player, newIndex);
						player.spwn = 1;
						player.elem = PT_DUST;
					}
					else if (partsptr[newIndex].type == PT_STKM2)
					{
						STKM_init_legs(&player2, newIndex);
						player2.spwn = 1;
						player2.elem = PT_DUST;
					}
					else if (partsptr[newIndex].type == PT_FIGH)
					{
						unsigned char fcount = 0;
						while (fcount < 100 && fcount < (fighcount+1) && fighters[fcount].spwn==1) fcount++;
						if (fcount < 100 && fighters[fcount].spwn==0)
						{
							partsptr[newIndex].tmp = fcount;
							fighters[fcount].spwn = 1;
							fighters[fcount].elem = PT_DUST;
							fighcount++;
							STKM_init_legs(&(fighters[fcount]), newIndex);
						}
					}
					if (partsptr[newIndex].type == PT_SOAP)
						partsptr[newIndex].ctype = 0; // delete all soap connections, but it looks like if tmp & tmp2 were saved to 3 bytes, connections would load properly
					if (!ptypes[partsptr[newIndex].type].enabled)
						partsptr[newIndex].type = PT_NONE;
					if (partsptr[newIndex].type == PT_MOVS)
					{
						if (partsptr[newIndex].life+oldnumballs < 256)
						{
							partsptr[newIndex].life += oldnumballs;
							msnum[partsptr[newIndex].life]++;
						}
						else
						{
							partsptr[newIndex].type = PT_NONE;
						}
						if (!partsptr[newIndex].tmp && !partsptr[newIndex].tmp2)
						{
							msindex[partsptr[newIndex].life] = newIndex+1;
							if(i >= partsDataLen) goto fail;
							msrotation[partsptr[newIndex].life] = partsData[i++]/20.0f - 6.283185307179586476925286766559;
						}
						if (partsptr[newIndex].tmp > 32768)
							partsptr[newIndex].tmp -= 65536;
						if (partsptr[newIndex].tmp2 > 32768)
							partsptr[newIndex].tmp2 -= 65536;
						if (partsptr[newIndex].type == PT_MOVS && !partsptr[newIndex].tmp && !partsptr[newIndex].tmp2)
						{
							partsData[partsDataLen++] = (int)(msrotation[partsptr[i].life] + 6.283185307179586476925286766559)*20;
						}
					}
					if (!ptypes[partsptr[newIndex].type].enabled && !secret_els)
						partsptr[newIndex].type = PT_NONE;
				}
			}
		}
	}
	goto fin;
fail:
	//Clean up everything
	returnCode = 1;
fin:
	bson_destroy(&b);
	if(freeIndices)
		free(freeIndices);
	return returnCode;
}

//Old saving
pixel *prerender_save_PSv(void *save, int size, int *width, int *height)
{
	unsigned char *d,*c=save,*m=NULL;
	int i,j,k,x,y,rx,ry,p=0;
	int bw,bh,w,h,new_format = 0;
	pixel *fb;

	if (size<16)
		return NULL;
	if (!(c[2]==0x43 && c[1]==0x75 && c[0]==0x66) && !(c[2]==0x76 && c[1]==0x53 && c[0]==0x50))
		return NULL;
	if (c[2]==0x43 && c[1]==0x75 && c[0]==0x66) {
		new_format = 1;
	}
	//if (c[4]>SAVE_VERSION)
	//	return NULL;

	bw = c[6];
	bh = c[7];
	w = bw*CELL;
	h = bh*CELL;

	if (c[5]!=CELL)
		return NULL;

	i = (unsigned)c[8];
	i |= ((unsigned)c[9])<<8;
	i |= ((unsigned)c[10])<<16;
	i |= ((unsigned)c[11])<<24;
	d = malloc(i);
	if (!d)
		return NULL;
	fb = calloc(w*h, PIXELSIZE);
	m = calloc(w*h, sizeof(int));
	if (!fb || !m)
	{
		free(d);
		return NULL;
	}

	if (BZ2_bzBuffToBuffDecompress((char *)d, (unsigned *)&i, (char *)(c+12), size-12, 0, 0))
		goto corrupt;
	size = i;

	if (size < bw*bh)
		goto corrupt;

	k = 0;
	for (y=0; y<bh; y++)
		for (x=0; x<bw; x++)
		{
			rx = x*CELL;
			ry = y*CELL;
			switch (d[p])
			{
			case 1:
				for (j=0; j<CELL; j++)
					for (i=0; i<CELL; i++)
						fb[(ry+j)*w+(rx+i)] = PIXPACK(0x808080);
				break;
			case 2:
				for (j=0; j<CELL; j+=2)
					for (i=(j>>1)&1; i<CELL; i+=2)
						fb[(ry+j)*w+(rx+i)] = PIXPACK(0x808080);
				break;
			case 3:
				for (j=0; j<CELL; j++)
					for (i=0; i<CELL; i++)
						if (!(j%2) && !(i%2))
							fb[(ry+j)*w+(rx+i)] = PIXPACK(0xC0C0C0);
				break;
			case 4:
				for (j=0; j<CELL; j+=2)
					for (i=(j>>1)&1; i<CELL; i+=2)
						fb[(ry+j)*w+(rx+i)] = PIXPACK(0x8080FF);
				k++;
				break;
			case 6:
				for (j=0; j<CELL; j+=2)
					for (i=(j>>1)&1; i<CELL; i+=2)
						fb[(ry+j)*w+(rx+i)] = PIXPACK(0xFF8080);
				break;
			case 7:
				for (j=0; j<CELL; j++)
					for (i=0; i<CELL; i++)
						if (!(i&j&1))
							fb[(ry+j)*w+(rx+i)] = PIXPACK(0x808080);
				break;
			case 8:
				for (j=0; j<CELL; j++)
					for (i=0; i<CELL; i++)
						if (!(j%2) && !(i%2))
							fb[(ry+j)*w+(rx+i)] = PIXPACK(0xC0C0C0);
						else
							fb[(ry+j)*w+(rx+i)] = PIXPACK(0x808080);
				break;
			case WL_WALL:
				for (j=0; j<CELL; j++)
					for (i=0; i<CELL; i++)
						fb[(ry+j)*w+(rx+i)] = PIXPACK(0x808080);
				break;
			case WL_DESTROYALL:
				for (j=0; j<CELL; j+=2)
					for (i=(j>>1)&1; i<CELL; i+=2)
						fb[(ry+j)*w+(rx+i)] = PIXPACK(0x808080);
				break;
			case WL_ALLOWLIQUID:
				for (j=0; j<CELL; j++)
					for (i=0; i<CELL; i++)
						if (!(j%2) && !(i%2))
							fb[(ry+j)*w+(rx+i)] = PIXPACK(0xC0C0C0);
				break;
			case WL_FAN:
				for (j=0; j<CELL; j+=2)
					for (i=(j>>1)&1; i<CELL; i+=2)
						fb[(ry+j)*w+(rx+i)] = PIXPACK(0x8080FF);
				k++;
				break;
			case WL_DETECT:
				for (j=0; j<CELL; j+=2)
					for (i=(j>>1)&1; i<CELL; i+=2)
						fb[(ry+j)*w+(rx+i)] = PIXPACK(0xFF8080);
				break;
			case WL_EWALL:
				for (j=0; j<CELL; j++)
					for (i=0; i<CELL; i++)
						if (!(i&j&1))
							fb[(ry+j)*w+(rx+i)] = PIXPACK(0x808080);
				break;
			case WL_WALLELEC:
				for (j=0; j<CELL; j++)
					for (i=0; i<CELL; i++)
						if (!(j%2) && !(i%2))
							fb[(ry+j)*w+(rx+i)] = PIXPACK(0xC0C0C0);
						else
							fb[(ry+j)*w+(rx+i)] = PIXPACK(0x808080);
				break;
			}
			p++;
		}
	p += 2*k;
	if (p>=size)
		goto corrupt;

	for (y=0; y<h; y++)
		for (x=0; x<w; x++)
		{
			if (p >= size)
				goto corrupt;
			j=d[p++];
			if (j<PT_NUM && j>0)
			{
				if (j==PT_STKM || j==PT_STKM2 || j==PT_FIGH)
				{
					pixel lc, hc=PIXRGB(255, 224, 178);
					if (j==PT_STKM || j==PT_FIGH) lc = PIXRGB(255, 255, 255);
					else lc = PIXRGB(100, 100, 255);
					//only need to check upper bound of y coord - lower bounds and x<w are checked in draw_line
					if (j==PT_STKM || j==PT_STKM2)
					{
						draw_line(fb , x-2, y-2, x+2, y-2, PIXR(hc), PIXG(hc), PIXB(hc), w);
						if (y+2<h)
						{
							draw_line(fb , x-2, y+2, x+2, y+2, PIXR(hc), PIXG(hc), PIXB(hc), w);
							draw_line(fb , x-2, y-2, x-2, y+2, PIXR(hc), PIXG(hc), PIXB(hc), w);
							draw_line(fb , x+2, y-2, x+2, y+2, PIXR(hc), PIXG(hc), PIXB(hc), w);
						}
					}
					else if (y+2<h)
					{
						draw_line(fb, x-2, y, x, y-2, PIXR(hc), PIXG(hc), PIXB(hc), w);
						draw_line(fb, x-2, y, x, y+2, PIXR(hc), PIXG(hc), PIXB(hc), w);
						draw_line(fb, x, y-2, x+2, y, PIXR(hc), PIXG(hc), PIXB(hc), w);
						draw_line(fb, x, y+2, x+2, y, PIXR(hc), PIXG(hc), PIXB(hc), w);
					}
					if (y+6<h)
					{
						draw_line(fb , x, y+3, x-1, y+6, PIXR(lc), PIXG(lc), PIXB(lc), w);
						draw_line(fb , x, y+3, x+1, y+6, PIXR(lc), PIXG(lc), PIXB(lc), w);
					}
					if (y+12<h)
					{
						draw_line(fb , x-1, y+6, x-3, y+12, PIXR(lc), PIXG(lc), PIXB(lc), w);
						draw_line(fb , x+1, y+6, x+3, y+12, PIXR(lc), PIXG(lc), PIXB(lc), w);
					}
				}
				else
					fb[y*w+x] = ptypes[j].pcolors;
				m[(x-0)+(y-0)*w] = j;
			}
		}
	for (j=0; j<w*h; j++)
	{
		if (m[j])
			p += 2;
	}
	for (j=0; j<w*h; j++)
	{
		if (m[j])
		{
			if (c[4]>=44)
				p+=2;
			else
				p++;
		}
	}
	if (c[4]>=44) {
		for (j=0; j<w*h; j++)
		{
			if (m[j])
			{
				p+=2;
			}
		}
	}
	if (c[4]>=53) {
		if (m[j]==PT_PBCN || m[j]==PT_MOVS || m[j]==PT_ANIM || ((m[j]==PT_PSCN || m[j]==PT_NSCN) && c[4] >= 240) || m[j]==PT_PPTI || m[j]==PT_PPTO || m[j]==PT_VIRS || m[j]==PT_VRSS || m[j]==PT_VRSG || (m[j]==PT_PCLN && c[4] >= 244))
			p++;
	}
	if (c[4]>=49) {
		for (j=0; j<w*h; j++)
		{
			if (m[j])
			{
				if (p >= size) {
					goto corrupt;
				}
				if (d[p++])
					fb[j] = PIXRGB(0,0,0);
			}
		}
		//Read RED component
		for (j=0; j<w*h; j++)
		{
			if (m[j])
			{
				if (p >= size) {
					goto corrupt;
				}
				if (m[j] <= NPART) {
					fb[j] |= PIXRGB(d[p++],0,0);
				} else {
					p++;
				}
			}
		}
		//Read GREEN component
		for (j=0; j<w*h; j++)
		{
			if (m[j])
			{
				if (p >= size) {
					goto corrupt;
				}
				if (m[j] <= NPART) {
					fb[j] |= PIXRGB(0,d[p++],0);
				} else {
					p++;
				}
			}
		}
		//Read BLUE component
		for (j=0; j<w*h; j++)
		{
			if (m[j])
			{
				if (p >= size) {
					goto corrupt;
				}
				if (m[j] <= NPART) {
					fb[j] |= PIXRGB(0,0,d[p++]);
				} else {
					p++;
				}
			}
		}
	}

	free(d);
	*width = w;
	*height = h;
	return fb;

corrupt:
	free(d);
	free(fb);
	return NULL;
}

void *build_save_PSv(int *size, int orig_x0, int orig_y0, int orig_w, int orig_h, unsigned char bmap[YRES/CELL][XRES/CELL], float fvx[YRES/CELL][XRES/CELL], float fvy[YRES/CELL][XRES/CELL], sign signs[MAXSIGNS], void* partsptr)
{
	unsigned char *d=calloc(1,3*(XRES/CELL)*(YRES/CELL)+(XRES*YRES)*(16+numframes*4)+MAXSIGNS*262+numballs*6), *c;
	int i,j,x,y,p=0,*m=calloc(XRES*YRES, sizeof(int)),nummovs = 0;
	int x0, y0, w, h, bx0=orig_x0/CELL, by0=orig_y0/CELL, bw, bh;
	particle *parts = partsptr;
	bw=(orig_w+orig_x0-bx0*CELL+CELL-1)/CELL;
	bh=(orig_h+orig_y0-by0*CELL+CELL-1)/CELL;

	// normalize coordinates
	x0 = bx0*CELL;
	y0 = by0*CELL;
	w  = bw *CELL;
	h  = bh *CELL;

	// save the required air state
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
			d[p++] = bmap[y][x];
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
			if (bmap[y][x]==WL_FAN||bmap[y][x]==4)
			{
				i = (int)(fvx[y][x]*64.0f+127.5f);
				if (i<0) i=0;
				if (i>255) i=255;
				d[p++] = i;
			}
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
			if (bmap[y][x]==WL_FAN||bmap[y][x]==4)
			{
				i = (int)(fvy[y][x]*64.0f+127.5f);
				if (i<0) i=0;
				if (i>255) i=255;
				d[p++] = i;
			}

	// save the particle map
	for (i=0; i<NPART; i++)
		if (parts[i].type)
		{
			x = (int)(parts[i].x+0.5f);
			y = (int)(parts[i].y+0.5f);
			if (x>=orig_x0 && x<orig_x0+orig_w && y>=orig_y0 && y<orig_y0+orig_h) {
				if (!m[(x-x0)+(y-y0)*w] || parts[m[(x-x0)+(y-y0)*w]-1].type == PT_PHOT || parts[m[(x-x0)+(y-y0)*w]-1].type == PT_NEUT)
					m[(x-x0)+(y-y0)*w] = i+1;
			}
		}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
			d[p++] = parts[i-1].type;
		else
			d[p++] = 0;
	}

	// save particle properties
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			i--;
			x = (int)(parts[i].vx*16.0f+127.5f);
			y = (int)(parts[i].vy*16.0f+127.5f);
			if (x<0) x=0;
			if (x>255) x=255;
			if (y<0) y=0;
			if (y>255) y=255;
			d[p++] = x;
			d[p++] = y;
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i) {
			//Everybody loves a 16bit int
			//d[p++] = (parts[i-1].life+3)/4;
			int ttlife = (int)parts[i-1].life;
			d[p++] = ((ttlife&0xFF00)>>8);
			d[p++] = (ttlife&0x00FF);
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i) {
			//Now saving tmp!
			//d[p++] = (parts[i-1].life+3)/4;
			int tttmp = (int)parts[i-1].tmp;
			d[p++] = ((tttmp&0xFF00)>>8);
			d[p++] = (tttmp&0x00FF);
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i && (parts[i-1].type==PT_PBCN || parts[i-1].type==PT_MOVS || parts[i-1].type==PT_ANIM || ((parts[i-1].type==PT_PSCN || parts[i-1].type==PT_NSCN || parts[i-1].type==PT_PCLN) && save_as == 0) || parts[i-1].type==PT_PPTI || parts[i-1].type==PT_PPTO || parts[i-1].type==PT_VIRS || parts[i-1].type==PT_VRSS || parts[i-1].type==PT_VRSG)) {
			//Save tmp2
			d[p++] = parts[i-1].tmp2;
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i) {
			//Save colour (ALPHA)
			d[p++] = (parts[i-1].dcolour&0xFF000000)>>24;
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i) {
			//Save colour (RED)
			d[p++] = (parts[i-1].dcolour&0x00FF0000)>>16;
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i) {
			//Save colour (GREEN)
			d[p++] = (parts[i-1].dcolour&0x0000FF00)>>8;
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i) {
			//Save colour (BLUE)
			d[p++] = (parts[i-1].dcolour&0x000000FF);
		}
	}
	if (save_as == 0)
	{
		i = pmap[4][4]>>8;
		if (parts[i].type == PT_INDI && parts[i].animations)
			parts[i].animations[1] = 0;
		for (j=0; j<w*h; j++)
		{
			i = m[j];
			if (i && parts[i-1].type==PT_ANIM)
			{
				int k;
				d[p++] = parts[i-1].ctype;
				for (k = 0; k <= parts[i-1].ctype;k++)
				{
					d[p++] = (parts[i-1].animations[k]&0xFF000000)>>24;
					d[p++] = (parts[i-1].animations[k]&0x00FF0000)>>16;
					d[p++] = (parts[i-1].animations[k]&0x0000FF00)>>8;
					d[p++] = (parts[i-1].animations[k]&0x000000FF);
				}
			}
			if (i && parts[i-1].type==PT_INDI)
			{
				int k;
				for (k = 0; k <= 256;k++)
				{
					d[p++] = (parts[i-1].animations[k]&0xFF000000)>>24;
					d[p++] = (parts[i-1].animations[k]&0x00FF0000)>>16;
					d[p++] = (parts[i-1].animations[k]&0x0000FF00)>>8;
					d[p++] = (parts[i-1].animations[k]&0x000000FF);
				}
			}
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			//New Temperature saving uses a 16bit unsigned int for temperatures, giving a precision of 1 degree versus 36 for the old format
			int tttemp = (int)parts[i-1].temp;
			d[p++] = ((tttemp&0xFF00)>>8);
			d[p++] = (tttemp&0x00FF);
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i && (parts[i-1].type==PT_CLNE || parts[i-1].type==PT_PCLN || parts[i-1].type==PT_BCLN || parts[i-1].type==PT_SPRK || parts[i-1].type==PT_LAVA || parts[i-1].type==PT_PIPE || parts[i-1].type==PT_LIFE || parts[i-1].type==PT_PBCN || parts[i-1].type==PT_WIRE || parts[i-1].type==PT_STOR || parts[i-1].type==PT_CONV))
			d[p++] = parts[i-1].ctype;
	}

	j = 0;
	for (i=0; i<MAXSIGNS; i++)
		if (signs[i].text[0] &&
		        signs[i].x>=x0 && signs[i].x<x0+w &&
		        signs[i].y>=y0 && signs[i].y<y0+h)
			j++;
	d[p++] = j;
	for (i=0; i<MAXSIGNS; i++)
		if (signs[i].text[0] &&
		        signs[i].x>=x0 && signs[i].x<x0+w &&
		        signs[i].y>=y0 && signs[i].y<y0+h)
		{
			d[p++] = (signs[i].x-x0);
			d[p++] = (signs[i].x-x0)>>8;
			d[p++] = (signs[i].y-y0);
			d[p++] = (signs[i].y-y0)>>8;
			d[p++] = signs[i].ju;
			x = strlen(signs[i].text);
			d[p++] = x;
			memcpy(d+p, signs[i].text, x);
			p+=x;
		}
	if (save_as == 0)
	{
		d[p++] =  decorations_enable|((aheat_enable<<1)&0x02)|((hud_enable<<2)&0x04)|((water_equal_test<<3)&0x08);//|((cmode<<4)&0xF0);
		for (j=0; j<w*h; j++)
		{
			i = m[j];
			if (i && parts[i-1].type==PT_MOVS && !parts[i].tmp && !parts[i].tmp2)
			{
				nummovs++;
			}
		}
		d[p++] = nummovs;
	}

	i = (p*101+99)/100 + 612;
	c = malloc(i);

	//New file header uses PSv, replacing fuC. This is to detect if the client uses a new save format for temperatures
	//This creates a problem for old clients, that display and "corrupt" error instead of a "newer version" error

	c[0] = 0x50;	//0x66;
	c[1] = 0x53;	//0x75;
	c[2] = 0x76;	//0x43;
	c[3] = legacy_enable|((sys_pause<<1)&0x02)|((gravityMode<<2)&0x0C)|((airMode<<4)&0x70)|((ngrav_enable<<7)&0x80);
	c[4] = saveversion;
	c[5] = CELL;
	c[6] = bw;
	c[7] = bh;
	c[8] = p;
	c[9] = p >> 8;
	c[10] = p >> 16;
	c[11] = p >> 24;

	i -= 12;

	if (BZ2_bzBuffToBuffCompress((char *)(c+12), (unsigned *)&i, (char *)d, p, 9, 0, 0) != BZ_OK)
	{
		free(d);
		free(c);
		free(m);
		return NULL;
	}
	free(d);
	free(m);

	*size = i+12;
	return c;
}

int parse_save_PSv(void *save, int size, int replace, int x0, int y0, unsigned char bmap[YRES/CELL][XRES/CELL], float fvx[YRES/CELL][XRES/CELL], float fvy[YRES/CELL][XRES/CELL], sign signs[MAXSIGNS], void* partsptr, unsigned pmap[YRES][XRES])
{
	unsigned char *d=NULL,*c=save;
	int q,i,j,k,x,y,p=0,*m=NULL, ver, pty, ty, legacy_beta=0, tempGrav = 0, modver = 0, oldnumballs = numballs;
	int bx0=x0/CELL, by0=y0/CELL, bw, bh, w, h;
	int nf=0, new_format = 0, ttv = 0;
	particle *parts = partsptr;
	int *fp = malloc(NPART*sizeof(int));

	//New file header uses PSv, replacing fuC. This is to detect if the client uses a new save format for temperatures
	//This creates a problem for old clients, that display and "corrupt" error instead of a "newer version" error

	if (size<16)
		return 1;
	if (!(c[2]==0x43 && c[1]==0x75 && c[0]==0x66) && !(c[2]==0x76 && c[1]==0x53 && c[0]==0x50))
		return 1;
	if (c[2]==0x76 && c[1]==0x53 && c[0]==0x50) {
		new_format = 1;
	}
	ver = c[4];
	if ((ver>SAVE_VERSION && ver < 200) || (ver < 237 && ver > 200+MOD_SAVE_VERSION))
		info_ui(vid_buf,"Save is from a newer version","Attempting to load it anyway, this may cause a crash");
	if (ver == 240) {
		ver = 65;
		modver = 3;
	}
	else if (ver == 242) {
		ver = 66;
		modver = 5;
	}
	else if (ver == 243) {
		ver = 68;
		modver = 6;
	}
	else if (ver == 244) {
		ver = 69;
		modver = 7;
	}
	else if (ver >= 200) {
		ver = 71;
		modver = 8;
	}

	if (ver<34)
	{
		legacy_enable = 1;
	}
	else
	{
		if (ver>=44) {
			legacy_enable = c[3]&0x01;
			if (!sys_pause) {
				sys_pause = (c[3]>>1)&0x01;
			}
			if (ver>=46 && replace) {
				gravityMode = ((c[3]>>2)&0x03);// | ((c[3]>>2)&0x01);
				airMode = ((c[3]>>4)&0x07);// | ((c[3]>>4)&0x02) | ((c[3]>>4)&0x01);
			}
			if (ver>=49 && replace) {
				tempGrav = ((c[3]>>7)&0x01);		
			}
		} else {
			if (c[3]==1||c[3]==0) {
				legacy_enable = c[3];
			} else {
				legacy_beta = 1;
			}
		}
	}

	bw = c[6];
	bh = c[7];
	if (bx0+bw > XRES/CELL)
		bx0 = XRES/CELL - bw;
	if (by0+bh > YRES/CELL)
		by0 = YRES/CELL - bh;
	if (bx0 < 0)
		bx0 = 0;
	if (by0 < 0)
		by0 = 0;

	if (c[5]!=CELL || bx0+bw>XRES/CELL || by0+bh>YRES/CELL)
		return 3;
	i = (unsigned)c[8];
	i |= ((unsigned)c[9])<<8;
	i |= ((unsigned)c[10])<<16;
	i |= ((unsigned)c[11])<<24;
	d = malloc(i);
	if (!d)
		return 1;

	if (BZ2_bzBuffToBuffDecompress((char *)d, (unsigned *)&i, (char *)(c+12), size-12, 0, 0))
		return 1;
	size = i;

	if (size < bw*bh)
		return 1;

	// normalize coordinates
	x0 = bx0*CELL;
	y0 = by0*CELL;
	w  = bw *CELL;
	h  = bh *CELL;

	if (replace)
	{
		if (ver<46) {
			gravityMode = 0;
			airMode = 0;
		}
		clear_sim();
	}
	if (modver >= 3 && replace)
	{
		memset(msindex, 0, sizeof(msindex));
		memset(msnum, 0, sizeof(msnum));
		memset(msvx, 0, sizeof(msvx));
		memset(msvy, 0, sizeof(msvy));
		memset(msrotation, 0, sizeof(msrotation));
		oldnumballs = 0;
	}
	parts_lastActiveIndex = NPART-1;
	m = calloc(XRES*YRES, sizeof(int));

	// make a catalog of free parts
	//memset(pmap, 0, sizeof(pmap)); "Using sizeof for array given as function argument returns the size of pointer."
	memset(pmap, 0, sizeof(unsigned)*(XRES*YRES));
	for (i=0; i<NPART; i++)
		if (parts[i].type)
		{
			x = (int)(parts[i].x+0.5f);
			y = (int)(parts[i].y+0.5f);
			pmap[y][x] = (i<<8)|1;
		}
		else
			fp[nf++] = i;

	// load the required air state
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
		{
			if (d[p])
			{
				//In old saves, ignore walls created by sign tool bug
				//Not ignoring other invalid walls or invalid walls in new saves, so that any other bugs causing them are easier to notice, find and fix
				if (ver<71 && d[p]==WL_SIGN)
				{
					p++;
					continue;
				}

				bmap[y][x] = d[p];
				if (bmap[y][x]==1)
					bmap[y][x]=WL_WALL;
				if (bmap[y][x]==2)
					bmap[y][x]=WL_DESTROYALL;
				if (bmap[y][x]==3)
					bmap[y][x]=WL_ALLOWLIQUID;
				if (bmap[y][x]==4)
					bmap[y][x]=WL_FAN;
				if (bmap[y][x]==5)
					bmap[y][x]=WL_STREAM;
				if (bmap[y][x]==6)
					bmap[y][x]=WL_DETECT;
				if (bmap[y][x]==7)
					bmap[y][x]=WL_EWALL;
				if (bmap[y][x]==8)
					bmap[y][x]=WL_WALLELEC;
				if (bmap[y][x]==9)
					bmap[y][x]=WL_ALLOWAIR;
				if (bmap[y][x]==10)
					bmap[y][x]=WL_ALLOWSOLID;
				if (bmap[y][x]==11)
					bmap[y][x]=WL_ALLOWALLELEC;
				if (bmap[y][x]==12)
					bmap[y][x]=WL_EHOLE;
				if (bmap[y][x]==13)
					bmap[y][x]=WL_ALLOWGAS;
			}

			p++;
		}
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
			if (d[(y-by0)*bw+(x-bx0)]==4||d[(y-by0)*bw+(x-bx0)]==WL_FAN)
			{
				if (p >= size)
					goto corrupt;
				fvx[y][x] = (d[p++]-127.0f)/64.0f;
			}
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
			if (d[(y-by0)*bw+(x-bx0)]==4||d[(y-by0)*bw+(x-bx0)]==WL_FAN)
			{
				if (p >= size)
					goto corrupt;
				fvy[y][x] = (d[p++]-127.0f)/64.0f;
			}

	// load the particle map
	i = 0;
	pty = p;
	for (y=y0; y<y0+h; y++)
		for (x=x0; x<x0+w; x++)
		{
			if (p >= size)
				goto corrupt;
			j=d[p++];
			if (j >= PT_NUM) {
				//TODO: Possibly some server side translation
				j = PT_DUST;//goto corrupt;
			}
			gol[x][y]=0;
			if (j)
			{
				if (modver > 0 && modver <= 5)
				{
					if (j >= 136 && j <= 140)
						j += (PT_NORMAL_NUM - 136);
					else if (j >= 142 && j <= 146)
						j += (PT_NORMAL_NUM - 137);
					d[p-1] = j;
				}
				if (pmap[y][x])
				{
					k = pmap[y][x]>>8;
				}
				else if (i<nf)
				{
					k = fp[i];
					i++;
				}
				else
				{
					m[(x-x0)+(y-y0)*w] = NPART+1;
					continue;
				}
				memset(parts+k, 0, sizeof(particle));
				parts[k].type = j;
				if (j == PT_COAL)
					parts[k].tmp = 50;
				if (j == PT_FUSE)
					parts[k].tmp = 50;
				if (j == PT_PHOT)
					parts[k].ctype = 0x3fffffff;
				if (j == PT_SOAP)
					parts[k].ctype = 0;
				if (j==PT_BIZR || j==PT_BIZRG || j==PT_BIZRS)
					parts[k].ctype = 0x47FFFF;
				parts[k].x = (float)x;
				parts[k].y = (float)y;
				m[(x-x0)+(y-y0)*w] = k+1;
			}
		}

	// load particle properties
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			i--;
			if (p+1 >= size)
				goto corrupt;
			if (i < NPART)
			{
				parts[i].vx = (d[p++]-127.0f)/16.0f;
				parts[i].vy = (d[p++]-127.0f)/16.0f;
			}
			else
				p += 2;
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			if (ver>=44) {
				if (p >= size) {
					goto corrupt;
				}
				if (i <= NPART) {
					ttv = (d[p++])<<8;
					ttv |= (d[p++]);
					parts[i-1].life = ttv;
					if (parts[i-1].type == PT_MOVS)
					{
						if (parts[i-1].life+oldnumballs < 256)
						{
							parts[i-1].life += oldnumballs;
							msnum[parts[i-1].life]++;
						}
						else
						{
							parts[i-1].type = PT_NONE;
						}
					}
				} else {
					p+=2;
				}
			} else {
				if (p >= size)
					goto corrupt;
				if (i <= NPART)
					parts[i-1].life = d[p++]*4;
				else
					p++;
			}
		}
	}
	if (ver>=44) {
		for (j=0; j<w*h; j++)
		{
			i = m[j];
			if (i)
			{
				if (p >= size) {
					goto corrupt;
				}
				if (i <= NPART) {
					ttv = (d[p++])<<8;
					ttv |= (d[p++]);
					parts[i-1].tmp = ttv;
					if (ver<53 && !parts[i-1].tmp)
						for (q = 1; q<=NGOLALT; q++) {
							if (parts[i-1].type==goltype[q-1] && grule[q][9]==2)
								parts[i-1].tmp = grule[q][9]-1;
						}
					if (ver>=51 && ver<53 && parts[i-1].type==PT_PBCN)
					{
						parts[i-1].tmp2 = parts[i-1].tmp;
						parts[i-1].tmp = 0;
					}
				} else {
					p+=2;
				}
			}
		}
	}
	if (ver>=53) {
		for (j=0; j<w*h; j++)
		{
			i = m[j];
			ty = d[pty+j];
			if (i && (ty==PT_PBCN || ty==PT_MOVS || ty==PT_ANIM || ((ty==PT_PSCN || ty==PT_NSCN) && modver >= 3) || ty==PT_PPTI || ty==PT_PPTO || ty==PT_VIRS || ty==PT_VRSS || ty==PT_VRSG || (ty==PT_PCLN && modver >= 7)))
			{
				if (p >= size)
					goto corrupt;
				if (i <= NPART)
				{
					parts[i-1].tmp2 = d[p++];
					if (parts[i-1].type == PT_MOVS && !parts[i-1].tmp && !parts[i-1].tmp2)
					{
						if (parts[i-1].life < 256)
							msindex[parts[i-1].life] = i;
					}
				}
				else
					p++;
			}
		}
	}
	//Read ALPHA component
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			if (ver>=49) {
				if (p >= size) {
					goto corrupt;
				}
				if (i <= NPART) {
					parts[i-1].dcolour = d[p++]<<24;
				} else {
					p++;
				}
			}
		}
	}
	//Read RED component
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			if (ver>=49) {
				if (p >= size) {
					goto corrupt;
				}
				if (i <= NPART) {
					parts[i-1].dcolour |= d[p++]<<16;
				} else {
					p++;
				}
			}
		}
	}
	//Read GREEN component
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			if (ver>=49) {
				if (p >= size) {
					goto corrupt;
				}
				if (i <= NPART) {
					parts[i-1].dcolour |= d[p++]<<8;
				} else {
					p++;
				}
			}
		}
	}
	//Read BLUE component
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			if (ver>=49) {
				if (p >= size) {
					goto corrupt;
				}
				if (i <= NPART) {
					parts[i-1].dcolour |= d[p++];
				} else {
					p++;
				}
			}
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i && parts[i-1].type == PT_ANIM)
		{
			if (modver>=3) {
				if (p >= size) {
					goto corrupt;
				}
				if (i <= NPART) {
					int k;
					parts[i-1].ctype = d[p++];
					if (parts[i-1].ctype > maxframes)
						maxframes = parts[i-1].ctype;
					parts[i-1].animations = calloc(maxframes,sizeof(unsigned int));
					if (parts[i-1].animations == NULL) {
						goto corrupt;
					}
					memset(parts[i-1].animations, 0, sizeof(parts[i-1].animations));
					for (k = 0; k <= parts[i-1].ctype; k++)
					{
						parts[i-1].animations[k] = d[p++]<<24;
						parts[i-1].animations[k] |= d[p++]<<16;
						parts[i-1].animations[k] |= d[p++]<<8;
						parts[i-1].animations[k] |= d[p++];
					}
				} else {
					p++;
				}
			}
		}
		if (i && parts[i-1].type == PT_INDI)
		{
			if (modver>=7) {
				if (p >= size) {
					goto corrupt;
				}
				if (i <= NPART) {
					int k;
					parts[i-1].animations = (unsigned int*)calloc(257,sizeof(unsigned int));
					if (parts[i-1].animations == NULL) {
						goto corrupt;
					}
					memset(parts[i-1].animations, 0, sizeof(parts[i-1].animations));
					for (k = 0; k <= 256; k++)
					{
						parts[i-1].animations[k] = d[p++]<<24;
						parts[i-1].animations[k] |= d[p++]<<16;
						parts[i-1].animations[k] |= d[p++]<<8;
						parts[i-1].animations[k] |= d[p++];
					}
				} else {
					p++;
				}
			}
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		ty = d[pty+j];
		if (i)
		{
			if (ver>=34&&legacy_beta==0)
			{
				if (p >= size)
				{
					goto corrupt;
				}
				if (i <= NPART)
				{
					if (ver>=42) {
						if (new_format) {
							ttv = (d[p++])<<8;
							ttv |= (d[p++]);
							if (parts[i-1].type==PT_PUMP) {
								parts[i-1].temp = ttv + 0.15;//fix PUMP saved at 0, so that it loads at 0.
							} else {
								parts[i-1].temp = (float)ttv;
							}
						} else {
							parts[i-1].temp = (float)(d[p++]*((MAX_TEMP+(-MIN_TEMP))/255))+MIN_TEMP;
						}
					} else {
						parts[i-1].temp = ((d[p++]*((O_MAX_TEMP+(-O_MIN_TEMP))/255))+O_MIN_TEMP)+273.0f;
					}
				}
				else
				{
					p++;
					if (new_format) {
						p++;
					}
				}
			}
			else
			{
				parts[i-1].temp = ptypes[parts[i-1].type].heat;
			}
		}
	} 
	for (j=0; j<w*h; j++)
	{
		int gnum = 0;
		i = m[j];
		ty = d[pty+j];
		if (i && (ty==PT_CLNE || (ty==PT_PCLN && ver>=43) || (ty==PT_BCLN && ver>=44) || (ty==PT_SPRK && ver>=21) || (ty==PT_LAVA && ver>=34) || (ty==PT_PIPE && ver>=43) || (ty==PT_LIFE && ver>=51) || (ty==PT_PBCN && ver>=52) || (ty==PT_WIRE && ver>=55) || (ty==PT_STOR && ver>=59) || (ty==PT_CONV && ver>=60)))
		{
			if (p >= size)
				goto corrupt;
			if (i <= NPART)
				parts[i-1].ctype = d[p++];
			else
				p++;
		}
		// no more particle properties to load, so we can change type here without messing up loading
		if (i && i<=NPART)
		{
			if ((player.spwn == 1 && ty==PT_STKM) || (player2.spwn == 1 && ty==PT_STKM2))
			{
				parts[i-1].type = PT_NONE;
			}
			else if (parts[i-1].type == PT_STKM)
			{
				STKM_init_legs(&player, i-1);
				player.spwn = 1;
				player.elem = PT_DUST;
			}
			else if (parts[i-1].type == PT_STKM2)
			{
				STKM_init_legs(&player2, i-1);
				player2.spwn = 1;
				player2.elem = PT_DUST;
			}
			else if (parts[i-1].type == PT_FIGH)
			{
				unsigned char fcount = 0;
				while (fcount < 100 && fcount < (fighcount+1) && fighters[fcount].spwn==1) fcount++;
				if (fcount < 100 && fighters[fcount].spwn==0)
				{
					parts[i-1].tmp = fcount;
					fighters[fcount].spwn = 1;
					fighters[fcount].elem = PT_DUST;
					fighcount++;
					STKM_init_legs(&(fighters[fcount]), i-1);
				}
			}

			if (ver<48 && (ty==OLD_PT_WIND || (ty==PT_BRAY&&parts[i-1].life==0)))
			{
				// Replace invisible particles with something sensible and add decoration to hide it
				x = (int)(parts[i-1].x+0.5f);
				y = (int)(parts[i-1].y+0.5f);
				parts[i-1].dcolour = 0xFF000000;
				parts[i-1].type = PT_DMND;
			}
			if(ver<51 && ((ty>=78 && ty<=89) || (ty>=134 && ty<=146 && ty!=141))){
				//Replace old GOL
				parts[i-1].type = PT_LIFE;
				for (gnum = 0; gnum<NGOLALT; gnum++){
					if (ty==goltype[gnum])
						parts[i-1].ctype = gnum;
				}
				ty = PT_LIFE;
			}
			if(ver<52 && (ty==PT_CLNE || ty==PT_PCLN || ty==PT_BCLN)){
				//Replace old GOL ctypes in clone
				for (gnum = 0; gnum<NGOLALT; gnum++){
					if (parts[i-1].ctype==goltype[gnum])
					{
						parts[i-1].ctype = PT_LIFE;
						parts[i-1].tmp = gnum;
					}
				}
			}
			if(ty==PT_LCRY){
				if(ver<67)
				{
					//New LCRY uses TMP not life
					if(parts[i-1].life>=10)
					{
						parts[i-1].life = 10;
						parts[i-1].tmp2 = 10;
						parts[i-1].tmp = 3;
					}
					else if(parts[i-1].life<=0)
					{
						parts[i-1].life = 0;
						parts[i-1].tmp2 = 0;
						parts[i-1].tmp = 0;
					}
					else if(parts[i-1].life < 10 && parts[i-1].life > 0)
					{
						parts[i-1].tmp = 1;
					}
				}
				else
				{
					parts[i-1].tmp2 = parts[i-1].life;
				}
			}
			if (!ptypes[parts[i-1].type].enabled)
				parts[i-1].type = PT_NONE;
		}
	}

	#ifndef RENDERER
	//Change the gravity state
	if(ngrav_enable != tempGrav && replace)
	{
		if(tempGrav)
			start_grav_async();
		else
			stop_grav_async();
	}
	#endif
	
	gravity_mask();

	if (p >= size)
		goto version1;
	j = d[p++];
	for (i=0; i<j; i++)
	{
		if (p+6 > size)
			goto corrupt;
		for (k=0; k<MAXSIGNS; k++)
			if (!signs[k].text[0])
				break;
		x = d[p++];
		x |= ((unsigned)d[p++])<<8;
		if (k<MAXSIGNS)
			signs[k].x = x+x0;
		x = d[p++];
		x |= ((unsigned)d[p++])<<8;
		if (k<MAXSIGNS)
			signs[k].y = x+y0;
		x = d[p++];
		if (k<MAXSIGNS)
			signs[k].ju = x;
		x = d[p++];
		if (p+x > size)
			goto corrupt;
		if (k<MAXSIGNS)
		{
			memcpy(signs[k].text, d+p, x);
			signs[k].text[x] = 0;
			clean_text(signs[k].text, 158-14 /* Current max sign length */);
		}
		p += x;
	}

	if (modver >= 3)
	{
		if (p >= size)
			goto version1;
		if (!decorations_enable) {
			decorations_enable = (d[p++])&0x01;
		}
		aheat_enable = (d[p]>>1)&0x01;
		hud_enable = (d[p]>>2)&0x01;
		water_equal_test = (d[p]>>3)&0x01;
		//if (replace)
		//	cmode = (d[p]>>4)&0x0F;
	}

	if (modver >= 3)
	{
		if (p >= size)
			goto version1;
		numballs += d[p++];
		for (i = 0; i < NPART; i++)
		{
			if (parts[i].type == PT_MOVS && parts[i].life >= oldnumballs && msindex[parts[i].life])
			{
				if (parts[i].x < parts[msindex[parts[i].life]-1].x)
				{
					parts[i].tmp -= 65536;
				}
				if (parts[i].y < parts[msindex[parts[i].life]-1].y)
				{
					parts[i].tmp2 -= 256;
				}
			}
		}
	}

version1:
	if (m) free(m);
	if (d) free(d);
	if (fp) free(fp);

	return 0;

corrupt:
	if (m) free(m);
	if (d) free(d);
	if (fp) free(fp);
	if (replace)
	{
		legacy_enable = 0;
		clear_sim();
	}
	return 1;
}

void *build_thumb(int *size, int bzip2)
{
	unsigned char *d=calloc(1,XRES*YRES), *c;
	int i,j,x,y;
	for (i=0; i<NPART; i++)
		if (parts[i].type)
		{
			x = (int)(parts[i].x+0.5f);
			y = (int)(parts[i].y+0.5f);
			if (x>=0 && x<XRES && y>=0 && y<YRES)
				d[x+y*XRES] = parts[i].type;
		}
	for (y=0; y<YRES/CELL; y++)
		for (x=0; x<XRES/CELL; x++)
			if (bmap[y][x])
				for (j=0; j<CELL; j++)
					for (i=0; i<CELL; i++)
						d[x*CELL+i+(y*CELL+j)*XRES] = 0xFF;
	j = XRES*YRES;

	if (bzip2)
	{
		i = (j*101+99)/100 + 608;
		c = malloc(i);

		c[0] = 0x53;
		c[1] = 0x68;
		c[2] = 0x49;
		c[3] = 0x74;
		c[4] = PT_NUM;
		c[5] = CELL;
		c[6] = XRES/CELL;
		c[7] = YRES/CELL;

		i -= 8;

		if (BZ2_bzBuffToBuffCompress((char *)(c+8), (unsigned *)&i, (char *)d, j, 9, 0, 0) != BZ_OK)
		{
			free(d);
			free(c);
			return NULL;
		}
		free(d);
		*size = i+8;
		return c;
	}

	*size = j;
	return d;
}

void *transform_save(void *odata, int *size, matrix2d transform, vector2d translate)
{
	void *ndata;
	unsigned char (*bmapo)[XRES/CELL] = calloc((YRES/CELL)*(XRES/CELL), sizeof(unsigned char));
	unsigned char (*bmapn)[XRES/CELL] = calloc((YRES/CELL)*(XRES/CELL), sizeof(unsigned char));
	particle *partst = calloc(sizeof(particle), NPART);
	sign *signst = calloc(MAXSIGNS, sizeof(sign));
	unsigned (*pmapt)[XRES] = calloc(YRES*XRES, sizeof(unsigned));
	float (*fvxo)[XRES/CELL] = calloc((YRES/CELL)*(XRES/CELL), sizeof(float));
	float (*fvyo)[XRES/CELL] = calloc((YRES/CELL)*(XRES/CELL), sizeof(float));
	float (*fvxn)[XRES/CELL] = calloc((YRES/CELL)*(XRES/CELL), sizeof(float));
	float (*fvyn)[XRES/CELL] = calloc((YRES/CELL)*(XRES/CELL), sizeof(float));
	float (*vxo)[XRES/CELL] = calloc((YRES/CELL)*(XRES/CELL), sizeof(float));
	float (*vyo)[XRES/CELL] = calloc((YRES/CELL)*(XRES/CELL), sizeof(float));
	float (*vxn)[XRES/CELL] = calloc((YRES/CELL)*(XRES/CELL), sizeof(float));
	float (*vyn)[XRES/CELL] = calloc((YRES/CELL)*(XRES/CELL), sizeof(float));
	float (*pvo)[XRES/CELL] = calloc((YRES/CELL)*(XRES/CELL), sizeof(float));
	float (*pvn)[XRES/CELL] = calloc((YRES/CELL)*(XRES/CELL), sizeof(float));
	int i, x, y, nx, ny, w, h, nw, nh;
	vector2d pos, tmp, ctl, cbr;
	vector2d cornerso[4];
	unsigned char *odatac = odata;
	if (parse_save(odata, *size, 0, 0, 0, bmapo, vxo, vyo, pvo, fvxo, fvyo, signst, partst, pmapt))
	{
		free(bmapo);
		free(bmapn);
		free(partst);
		free(signst);
		free(pmapt);
		free(fvxo);
		free(fvyo);
		free(fvxn);
		free(fvyn);
		free(vxo);
		free(vyo);
		free(vxn);
		free(vyn);
		free(pvo);
		free(pvn);
		return odata;
	}
	w = odatac[6]*CELL;
	h = odatac[7]*CELL;
	// undo any translation caused by rotation
	cornerso[0] = v2d_new(0,0);
	cornerso[1] = v2d_new(w-1.0f,0.0f);
	cornerso[2] = v2d_new(0.0f,h-1.0f);
	cornerso[3] = v2d_new(w-1.0f,h-1.0f);
	for (i=0; i<4; i++)
	{
		tmp = m2d_multiply_v2d(transform,cornerso[i]);
		if (i==0) ctl = cbr = tmp; // top left, bottom right corner
		if (tmp.x<ctl.x) ctl.x = tmp.x;
		if (tmp.y<ctl.y) ctl.y = tmp.y;
		if (tmp.x>cbr.x) cbr.x = tmp.x;
		if (tmp.y>cbr.y) cbr.y = tmp.y;
	}
	// casting as int doesn't quite do what we want with negative numbers, so use floor()
	tmp = v2d_new(floor(ctl.x+0.5f),floor(ctl.y+0.5f));
	translate = v2d_sub(translate,tmp);
	nw = (int)(floor(cbr.x+0.5f)-floor(ctl.x+0.5f)+1);
	nh = (int)(floor(cbr.y+0.5f)-floor(ctl.y+0.5f)+1);
	if (nw>XRES) nw = XRES;
	if (nh>YRES) nh = YRES;
	// rotate and translate signs, parts, walls
	for (i=0; i<MAXSIGNS; i++)
	{
		if (!signst[i].text[0]) continue;
		pos = v2d_new((float)signst[i].x, (float)signst[i].y);
		pos = v2d_add(m2d_multiply_v2d(transform,pos),translate);
		nx = (int)floor(pos.x+0.5f);
		ny = (int)floor(pos.y+0.5f);
		if (nx<0 || nx>=nw || ny<0 || ny>=nh)
		{
			signst[i].text[0] = 0;
			continue;
		}
		signst[i].x = nx;
		signst[i].y = ny;
	}
	for (i=0; i<NPART; i++)
	{
		if (!partst[i].type) continue;
		pos = v2d_new(partst[i].x, partst[i].y);
		pos = v2d_add(m2d_multiply_v2d(transform,pos),translate);
		nx = (int)floor(pos.x+0.5f);
		ny = (int)floor(pos.y+0.5f);
		if (nx<0 || nx>=nw || ny<0 || ny>=nh)
		{
			partst[i].type = PT_NONE;
			continue;
		}
		partst[i].x = (float)nx;
		partst[i].y = (float)ny;
	}
	for (y=0; y<YRES/CELL; y++)
		for (x=0; x<XRES/CELL; x++)
		{
			pos = v2d_new(x*CELL+CELL*0.4f, y*CELL+CELL*0.4f);
			pos = v2d_add(m2d_multiply_v2d(transform,pos),translate);
			nx = (int)(pos.x/CELL);
			ny = (int)(pos.y/CELL);
			if (nx<0 || nx>=nw/CELL || ny<0 || ny>=nh/CELL)
				continue;
			if (bmapo[y][x])
			{
				bmapn[ny][nx] = bmapo[y][x];
				if (bmapo[y][x]==WL_FAN)
				{
					fvxn[ny][nx] = fvxo[y][x];
					fvyn[ny][nx] = fvyo[y][x];
				}
			}
			vxn[ny][nx] = vxo[y][x];
			vyn[ny][nx] = vyo[y][x];
			pvn[ny][nx] = pvo[y][x];
		}
	ndata = build_save(size,0,0,nw,nh,bmapn,vxn,vyn,pvn,fvxn,fvyn,signst,partst);
	free(bmapo);
	free(bmapn);
	free(partst);
	free(signst);
	free(pmapt);
	free(fvxo);
	free(fvyo);
	free(fvxn);
	free(fvyn);
	free(vxo);
	free(vyo);
	free(vxn);
	free(vyn);
	free(pvo);
	free(pvn);
	return ndata;
}
