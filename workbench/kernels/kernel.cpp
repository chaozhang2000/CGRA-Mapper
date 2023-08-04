#define NTAPS 32

int input[NTAPS]={1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,
};
int output[NTAPS*2-1];
int coefficients[NTAPS] = {0, 1, 3, -2, 0, 0, -3, 1,
0, 1, 3, -2, 0, 0, -3, 1,
0, 1, 3, -2, 0, 0, -3, 1,
0, 1, 3, -2, 0, 0, -3, 1};

void kernel(int input[], int output[], int coefficient[]);

int main()
{

//  input_dsp (input, NTAPS, 0);

  kernel(input, output, coefficients);

//  output_dsp (input, NTAPS, 0);
//  output_dsp (coefficients, NTAPS, 0);
//  output_dsp (output, NTAPS, 0);
  return 0;
}

void kernel(int input[], int output[], int coefficient[])
/*   input :           input sample array */
/*   output:           output sample array */
/*   coefficient:      coefficient array */
{
  int i;
  int j = 0;

  for(j=0; j< 63; j++) {
    for (i = 0; i < NTAPS; i++) {
	if(j-i>=0){
      output[j] += input[j-i] * coefficient[i];
	}
 	}
  }
}
