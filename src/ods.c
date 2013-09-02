/**
 * ods.c: Exposes OutputDebugString to MATLAB via MEX.
 **
 * (c) 2013 Christopher E. Granade (cgranade@cgranade.com).
 *    
 * This file is a part of the pymex-embed project.
 * Licensed under the AGPL version 3.
 **
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

// INCLUDES ////////////////////////////////////////////////////////////////////

#include <mex.h>
#ifdef WINDOWS
    #include <Windows.h>
    #pragma comment(lib, "Kernel32.lib");
#endif

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    char* c_str;
    mxArray *m_str;
    int buf_size, status;
    
    if (prhs == NULL) {
        mexErrMsgTxt("prhs was NULL.");
    }
    m_str = prhs[0];
    
    buf_size = (mxGetM(m_str) * mxGetN(m_str)) + 1;
    c_str = mxCalloc(buf_size, sizeof(char));
    
    // Attempt to convert the MATLAB string into a C string.
    status = mxGetString(m_str, *c_str, buf_size);
    if (status != 0) {
        mexErrMsgTxt("Error while converting string.");
    }
    
    OutputDebugString(c_str);
}