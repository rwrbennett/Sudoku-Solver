
#include <string.h>
#include <stdlib.h> 

extern "C"
{
	typedef void (__stdcall * ProgressCallback)(unsigned char *);

	__declspec(dllexport) void solve(unsigned char *buffer, int nSize);
	__declspec(dllexport) void showSteps(unsigned char *buffer, int nSize, ProgressCallback progressCallback);	
}


ProgressCallback callback = NULL;

void solvePuzzle(unsigned char *buffer, int nSize);


typedef struct square
{
	unsigned short value;
	unsigned short rowValues;
	unsigned short colValues;
	unsigned short boxValues;
	unsigned short possibleValues;	//Possible values for empty squares
	unsigned short possibleValuesCount;	//Number of bits that are set in pos (number of possible values)
	unsigned short row_index;	//The row that this square belongs to (0-8)
	unsigned short col_index;	//The column that this square belongs to (0-8)
	unsigned short box_index;	//The box that this square belongs to (0-8)	
} _square;


int totalValuesSet = 0; //Used to display number of attempts to set values (information only).


unsigned short bitCount (unsigned short value) 
{
    unsigned short count = 0;
    while (value > 0) {           // until all bits are zero
        if ((value & 1) == 1)     // check lower bit
            count++;
        value >>= 1;              // shift bits, removing lower bit
    }
    return count;
}

bool isBitSet (unsigned short bit, unsigned short value) 
{
	return ((value & (1<<bit)) ? true: false);
}

void updateValues(_square *squareArray)
{
	int i;
	int j;

	for (i=0; i<81; i++)
	{
		squareArray[i].rowValues = 0;
		squareArray[i].colValues = 0;
		squareArray[i].boxValues = 0;

		for (j=0; j<81; j++)
		{
			if (i==j) continue;
			if (squareArray[i].row_index == squareArray[j].row_index)
				squareArray[i].rowValues |= (squareArray[j].value > 0) ? (1 << (squareArray[j].value-1)) : 0;

			if (squareArray[i].col_index == squareArray[j].col_index)
				squareArray[i].colValues |= (squareArray[j].value > 0) ? (1 << (squareArray[j].value-1)) : 0;

			if (squareArray[i].box_index == squareArray[j].box_index)
				squareArray[i].boxValues |= (squareArray[j].value > 0) ? (1 << (squareArray[j].value-1)) : 0;

		}

		if (squareArray[i].value == 0)
		{
			squareArray[i].possibleValues = (~(squareArray[i].rowValues | squareArray[i].colValues | squareArray[i].boxValues)) & 0x1FF;
			squareArray[i].possibleValuesCount = bitCount(squareArray[i].possibleValues);
		}
		else
		{
			squareArray[i].possibleValues = 0;
			squareArray[i].possibleValuesCount = 0;
		}
	}

}



bool setSinglePossibleValue(_square *squareArray)
{
	bool valueSet = false;

	int i;

	for (i=0; i<81; i++)
	{			
			if (squareArray[i].value == 0)
			{
				if (squareArray[i].possibleValuesCount == 1) //There is only one possible value for this square
				{
					unsigned short val = squareArray[i].possibleValues;
					for (int bit=0; bit<9; bit++)
					{
						if (val & (1 << bit)) 
						{
							squareArray[i].value =bit+1; //Set the value of the square
							totalValuesSet++;
							break; //No need to check the rest of the bits since we know only one is set
						}
					}
					
					valueSet = true;

					break; //Only set one value, then update possible values for all other squares
				}
			}		
	}		
	return valueSet;
}


//If a value cannot go in any other square within the current box, row, or column, it must go in this square

bool setForcedValue(_square *squareArray)
{
	int i;
	int j;

	bool valueSet = false;
	
	for (i=0; i<81; i++) 
	{
		if (squareArray[i].value == 0)
		{		
			unsigned short possibleValues = squareArray[i].possibleValues;
			unsigned short targetValue = 0;
			bool found = false;
			int bit;				
			
				for (bit=0; bit<9; bit++)
				{					
					int val = possibleValues  & (1 << bit);
					
					if (val) 
					{
						found = false;
						for (j=0; j<81; j++)
						{																						
							if (squareArray[i].box_index == squareArray[j].box_index)
							{
								if (i != j)
								{
									if (squareArray[j].possibleValues & val)
									{
										found = true;
										break;
									}
								}
							}
						}		

						//If we have checked all the other squares in this box 
						//and the specified value is not valid in any of them, it must go in this square									
						if (!found)
						{
							targetValue = bit+1;							
						}
						else //Check rows and columns
						{
							found = false;
							for (j=0; j<81; j++)
							{																						
								if (squareArray[i].row_index == squareArray[j].row_index)
								{
									if (i != j)
									{
										if (squareArray[j].possibleValues & val)
										{
											found = true;
											break;
										}
									}
								}
							}		

							//If we have checked all the other squares in this row 
							//and the specified value is not valid in any of them, it must go in this square									
							if (!found)
							{								
								targetValue = bit+1;
							}
							else
							{
								found = false;
								
								for (j=0; j<81; j++)
								{																						
									if (squareArray[i].col_index == squareArray[j].col_index)
									{
										if (i != j)
										{
											if (squareArray[j].possibleValues & val)
											{
												found = true;
												break; 
											}
										}
									}
								}		

								//If we have checked all the other squares in this column
								//and the specified value is not valid in any of them, it must go in this square									
								if (!found)
								{
									targetValue = bit+1;									
								}

							}
						}
					}
				}
		
				if (targetValue)
				{
					squareArray[i].value = targetValue; //Set the value of the square								
					totalValuesSet++;
					valueSet = true;
					break;
				}

			}
		}
	return valueSet;

}


int SetValues(_square *squareArray, int emptySquareCount)
{

	bool valueSet = true;
	
	while (valueSet)
	{		
		updateValues(squareArray);

		valueSet = setSinglePossibleValue(squareArray);

		if (valueSet)
		{
			emptySquareCount--;	
			continue;
		}

		valueSet = setForcedValue(squareArray);

		if (valueSet) 
		{	
			emptySquareCount--;
		}
				
	}	

	return emptySquareCount;

}

bool readValuesFromArray(_square *squareArray, unsigned char *buffer, int *emptySquareCount)
{
	
	int i;
	int squareIndex = 0;

	*emptySquareCount = 0;


	//Read 81 characters from the array

	for (i=0; i<81; i++)
	{

		squareArray[squareIndex].value = buffer[i];

		if (squareArray[squareIndex].value == 0)
			(*emptySquareCount)++;
		
		squareIndex++;
	}	
	
	return (squareIndex == 81) ? true : false;

}


int *getPossibleValuesForSquare(int possibleValuesCount, int possibleValues)
{

	int *possibleValuesArray = (int *) malloc(possibleValuesCount * sizeof(int));
	for (int j=0; j<possibleValuesCount; j++)
	{
		for (int k=0; k<9; k++)
		{
			if (isBitSet(k, possibleValues))
				possibleValuesArray[j++] = k+1;
		}
	}
	return possibleValuesArray;
}

void InitializeArray(_square *squareArray)
{
	int i;
	int j;
	int squareIndex = 0;
			
	for (i=0; i<9; i++)
	{
		for (j=0; j<9; j++)
		{			

			squareArray[squareIndex].row_index = i;
			squareArray[squareIndex].col_index = j;			
			squareArray[squareIndex].value = 0;
			squareArray[squareIndex].rowValues = 0;
			squareArray[squareIndex].colValues = 0;
			squareArray[squareIndex].boxValues = 0;
			squareArray[squareIndex].possibleValues = 0;			
			squareArray[squareIndex].possibleValuesCount = 0;
			
			if (j<3)
			{
				if (i<3)
					squareArray[squareIndex].box_index = 0;
				else if (i>5)
					squareArray[squareIndex].box_index = 2;
				else
					squareArray[squareIndex].box_index = 1;
			}
			else if (j>5)
			{
				if (i<3)
					squareArray[squareIndex].box_index = 6;
				else if (i>5)
					squareArray[squareIndex].box_index = 8;
				else
					squareArray[squareIndex].box_index = 7;
			}
			else
			{
				if (i<3)
					squareArray[squareIndex].box_index = 3;
				else if (i>5)
					squareArray[squareIndex].box_index = 5;
				else
					squareArray[squareIndex].box_index = 4;
			}
			squareIndex++;
		}

	}


}

void SendCallback(_square *squareArray				  )
{
	unsigned char updatedBuffer[81];

	if (callback == NULL)
		return;

	memset(updatedBuffer, 0, 81);

	for (int i=0; i<81; i++)
	{
		updatedBuffer[i] = squareArray[i].value;
	}

	callback(updatedBuffer);

}

__declspec(dllexport) void solve(unsigned char *buffer, int nSize)
{
	callback = NULL;
	solvePuzzle(buffer, nSize);

}

__declspec(dllexport) void showSteps(unsigned char *buffer, int nSize, ProgressCallback progressCallback)
{
	callback = progressCallback;
	solvePuzzle(buffer, nSize);
}

void solvePuzzle(unsigned char *buffer, int nSize)
{
	
	int i;
	int j;
	int k;
	int l;
	int emptySquareCount = 82;	
	int emptySquareCount2 = 82;	
	int count = 82;
	_square squareArray[81];
	_square backupArray[81]; //Array state before trying trial value
	_square backupArray2[81]; //Array state before trying second trial value
	bool solved = false;

	if (nSize < 81)
	{
		return;
	}

		totalValuesSet = 0;
		solved = false;
		InitializeArray(squareArray);

		if (!readValuesFromArray(squareArray, buffer, &emptySquareCount))
		{
			return;
		}
		if (emptySquareCount > 64)
		{	
			return;
		}


		emptySquareCount = SetValues(squareArray, emptySquareCount);

		SendCallback(squareArray);

		if (emptySquareCount > 0)
		{

			//At this point, if we haven't set all square values we need to start trying possible values,
			//then setting values that appear to be the only possible ones
			//Continue until we hit a dead-end, roll-back and try another possible value.

			memcpy(backupArray, squareArray, (81*sizeof(_square)));	

			for (i=0; i<81; i++)
			{
				int possibleValuesCount = squareArray[i].possibleValuesCount;

				if (possibleValuesCount > 0)
				{
					int *possibleValues = getPossibleValuesForSquare(possibleValuesCount, squareArray[i].possibleValues);

					//Set the square value to each value in turn and attempt to solve
					for (j=0;j<possibleValuesCount; j++)
					{
						squareArray[i].value = possibleValues[j];
						squareArray[i].possibleValues = 0;
						squareArray[i].possibleValuesCount = 0;
						emptySquareCount--;
						
						count = SetValues(squareArray, emptySquareCount);
						SendCallback(squareArray);
						if (count > 0)
						{	
							//Rollback to the last know good state and try the next possible value
							memcpy(squareArray, backupArray, (81*sizeof(_square)));						
							emptySquareCount++;
							SendCallback(squareArray);
						}
						else
						{
							emptySquareCount = 0;
							solved=true;
							break;
						}					
					}
					free(possibleValues);
					possibleValues = NULL;
				}
				if (solved) 
				{							
					break;
				}

			}

			if (emptySquareCount > 0)
			{
				memcpy(backupArray, squareArray, (81*sizeof(_square)));	

				for (i=0; i<81; i++)
				{
					int possibleValuesCount = squareArray[i].possibleValuesCount;
					if (possibleValuesCount > 0)
					{
						int *possibleValues = getPossibleValuesForSquare(possibleValuesCount, squareArray[i].possibleValues);

						//Set the square value to each value in turn and attempt to solve
						for (j=0;j<possibleValuesCount; j++)
						{
							squareArray[i].value = possibleValues[j];
							squareArray[i].possibleValues = 0;
							squareArray[i].possibleValuesCount = 0;
							emptySquareCount--;
												
							emptySquareCount2 = SetValues(squareArray, emptySquareCount);
							//We know we were not able to solve using a single trial value since we tried that above
							//so go straight to setting a second trial value							
							SendCallback(squareArray);

							for (k=0; k<81; k++)
							{
								if (k!=i)
								{
									memcpy(backupArray2, squareArray, (81*sizeof(_square)));										
									int possibleValuesCount2 = squareArray[k].possibleValuesCount;
									if (possibleValuesCount2 > 0)
									{
										int *possibleValues2 = getPossibleValuesForSquare(possibleValuesCount2, squareArray[k].possibleValues);

										for (l=0;l<possibleValuesCount2; l++)
										{
											squareArray[k].value = possibleValues2[l];
											squareArray[k].possibleValues = 0;
											squareArray[k].possibleValuesCount = 0;
											emptySquareCount2--;

											count = SetValues(squareArray, emptySquareCount2);
											SendCallback(squareArray);
											if (count > 0)
											{	
												//Rollback to the state before setting the second trial value
												memcpy(squareArray, backupArray2, (81*sizeof(_square)));											
												emptySquareCount2++;
												SendCallback(squareArray);
											}
											else
											{
												emptySquareCount = 0;
												solved = true;
												break;
											}

										}
										free(possibleValues2);
										possibleValues2 = NULL;

									}
									if (solved) 
									{							
										break;
									}
								}

							}
							if (solved) 
							{							
								break;
							}

							//Tried all possible second trial values
							//Rollback to the state before setting the first trial value							
							memcpy(squareArray, backupArray, (81*sizeof(_square)));						
							emptySquareCount++;				
							SendCallback(squareArray);
						}

						free(possibleValues);
						possibleValues = NULL;
					}
					if (solved) 
					{					
						break;
					}

				}
				SendCallback(squareArray);
			}

		}

	if (callback == NULL)
	{
		for (int i=0; i<nSize; i++)
			buffer[i] = squareArray[i].value;
	}


	return;
}


