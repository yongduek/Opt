#include "CUDAWarpingSolver.h"

extern "C" void ImageWarpiungSolveGNStub(SolverInput& input, SolverState& state, SolverParameters& parameters);	// gauss newton

CUDAWarpingSolver::CUDAWarpingSolver(unsigned int imageWidth, unsigned int imageHeight) : m_imageWidth(imageWidth), m_imageHeight(imageHeight)
{
	const unsigned int THREADS_PER_BLOCK = 1024; // keep consistent with the GPU
	const unsigned int tmpBufferSize = THREADS_PER_BLOCK*THREADS_PER_BLOCK;

	const unsigned int N = m_imageWidth*m_imageHeight;
	const unsigned int numberOfVariables = N;

	// State
	cutilSafeCall(cudaMalloc(&m_solverState.d_delta,		sizeof(float3)*numberOfVariables));
	cutilSafeCall(cudaMalloc(&m_solverState.d_r,			sizeof(float3)*numberOfVariables));
	cutilSafeCall(cudaMalloc(&m_solverState.d_z,			sizeof(float3)*numberOfVariables));
	cutilSafeCall(cudaMalloc(&m_solverState.d_p,			sizeof(float3)*numberOfVariables));
	cutilSafeCall(cudaMalloc(&m_solverState.d_Ap_X,			sizeof(float3)*numberOfVariables));
	cutilSafeCall(cudaMalloc(&m_solverState.d_scanAlpha,	sizeof(float)*tmpBufferSize));
	cutilSafeCall(cudaMalloc(&m_solverState.d_scanBeta,		sizeof(float)*tmpBufferSize));
	cutilSafeCall(cudaMalloc(&m_solverState.d_rDotzOld,		sizeof(float)*numberOfVariables));
	cutilSafeCall(cudaMalloc(&m_solverState.d_precondioner, sizeof(float3)*numberOfVariables));
	cutilSafeCall(cudaMalloc(&m_solverState.d_sumResidual,	sizeof(float)));
}

CUDAWarpingSolver::~CUDAWarpingSolver()
{
	// State
	cutilSafeCall(cudaFree(m_solverState.d_delta));
	cutilSafeCall(cudaFree(m_solverState.d_r));
	cutilSafeCall(cudaFree(m_solverState.d_z));
	cutilSafeCall(cudaFree(m_solverState.d_p));
	cutilSafeCall(cudaFree(m_solverState.d_Ap_X));
	cutilSafeCall(cudaFree(m_solverState.d_scanAlpha));
	cutilSafeCall(cudaFree(m_solverState.d_scanBeta));
	cutilSafeCall(cudaFree(m_solverState.d_rDotzOld));
	cutilSafeCall(cudaFree(m_solverState.d_precondioner));
	cutilSafeCall(cudaFree(m_solverState.d_sumResidual));
}

void CUDAWarpingSolver::solveGN(float3* d_image, float3* d_target, unsigned int nNonLinearIterations, unsigned int nLinearIterations, float weightFitting, float weightRegularizer, float pNorm)
{
	m_solverState.d_target = d_target;
	m_solverState.d_x = d_image;

	SolverParameters parameters;
	parameters.weightFitting = weightFitting;
	parameters.weightRegularizer = weightRegularizer;	
	parameters.nNonLinearIterations = nNonLinearIterations;
	parameters.nLinIterations = nLinearIterations;
	parameters.pNorm = pNorm;
	
	SolverInput solverInput;
	solverInput.N = m_imageWidth*m_imageHeight;
	solverInput.width = m_imageWidth;
	solverInput.height = m_imageHeight;

	ImageWarpiungSolveGNStub(solverInput, m_solverState, parameters);
}
