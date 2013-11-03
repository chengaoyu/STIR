 /*
* Copyright (c) 2013,
* Biomedical Image Group (GIB), Universitat de Barcelona, Barcelona, Spain. All rights reserved.
* This software is distributed WITHOUT ANY WARRANTY; 
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  \author Carles Falcon

*/

#ifndef _WEIGHT3D_SPECTUB_H
#define _WEIGHT3D_SPECTUB_H

/* Global variables: the matrix etc TODO 
   Need to be defined elsewhere

   A lot of the functions below modify these variables.
*/
extern wm_da_type wm;
extern wmh_type wmh; 
extern float * Rrad;

void wm_calculation( const int kOS,
					const angle_type *const ang, 
					voxel_type vox, 
					bin_type bin, 
					const volume_type& vol, 
					const proj_type& prj, 
					const float *attmap,
					const bool *msk_3d,
					const bool *msk_2d,
					const int maxszb,
					const discrf_type *const gaussdens,
					const int *const  NITEMS
					);

void wm_size_estimation (int kOS,
						 const angle_type * const ang, 
						 voxel_type vox, 
						 bin_type bin, 
						 const volume_type& vol, 
						 const proj_type& prj, 
						 const bool * const msk_3d,
						 const bool *const msk_2d,
						 const int maxszb,
						 const discrf_type * const gaussdens,
						 int *NITEMS);


//... geometric component ............................................

void calc_gauss ( discrf_type *gaussdens );             

void calc_vxprj ( angle_type *ang );

void voxel_projection ( voxel_type *vox, float * eff, float lngcmd2 );

void fill_psf_no( psf_type *psf, const voxel_type& vox, const angle_type * const ang, float szdx );

void fill_psf_2d( psf_type *psf, const voxel_type& vox, discrf_type const*const gaussdens, float szdx );

void fill_psf_3d( psf_type *psf, const voxel_type& vox, discrf_type const * const gaussdens, float szdx, float thdx, float thcmd2 );

void calc_psf_bin ( float center_psf, float binszcm, discrf_type const * const vxprj, psf_type *psf );


//... attenuation...................................................

// not used
//void size_attpth_simple( psf_type *psf, voxel_type vox, volume_type vol, float *att, angle_type *ang );

//void size_attpth_full( psf_type *psf, voxel_type vox, volume_type vol, float *att, angle_type *ang );

void calc_att_path ( const bin_type& bin, const voxel_type& vox, const volume_type& vol, attpth_type *attpth );

float calc_att ( const attpth_type *const attpth, const float *const attmap, int islc );

int comp_dist ( float dx, float dy, float dz, float dlast );


void error_weight3d ( int nerr, const std::string& txt);               // error messages in weight3d_SPECT

#endif
