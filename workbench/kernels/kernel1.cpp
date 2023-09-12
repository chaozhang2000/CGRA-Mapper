#define NTAPS 32

float input[NTAPS];
float output[NTAPS];
float coefficients[NTAPS] = {0.25, 1.50, 3.75, -2.25, 0.50, 0.75, -3.00, 1.25,
0.25, 1.50, 3.75, -2.25, 0.50, 0.75, -3.00, 1.25,
0.25, 1.50, 3.75, -2.25, 0.50, 0.75, -3.00, 1.25,
0.25, 1.50, 3.75, -2.25, 0.50, 0.75, -3.00, 1.25};

void kernel(float input[], float output[], float coefficient[],long i,long j);

int main()
{

//  input_dsp (input, NTAPS, 0);

	long i;
	long j=0;
	for(i=0;i<NTAPS;++i){
  kernel(input, output, coefficients,i,j);
	}
//  output_dsp (input, NTAPS, 0);
//  output_dsp (coefficients, NTAPS, 0);
//  output_dsp (output, NTAPS, 0);
  return 0;
}

void kernel(float input[], float output[], float coefficient[],long i,long j)
/*   input :           input sample array */
/*   output:           output sample array */
/*   coefficient:      coefficient array */
{

//  for(j=0; j< NTAPS; ++j) {
			if(i<j){
      output[j] += input[i] * coefficient[i];
			}
			else{
			output[j]+= input[i]/coefficient[i];
			}
//  }
}
