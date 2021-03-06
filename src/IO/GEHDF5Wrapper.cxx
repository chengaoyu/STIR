//
//
/*!
  \file
  \ingroup IO
  \ingroup GE
  \brief Declaration of class stir::GE::RDF_HDF5::GEHDF5Wrapper

  \author Nikos Efthimiou
  \author Palak Wadhwa

*/
/*
    Copyright (C) 2017-2019, University of Leeds
    Copyright (C) 2018 University of Hull
    Copyright (C) 2018-2019, University College London
    This file is part of STIR.

    This file is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.

    This file is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    See STIR/LICENSE.txt for details
*/

#include "stir/IO/GEHDF5Wrapper.h"
#include <sstream>

START_NAMESPACE_STIR
namespace GE {
namespace RDF_HDF5 {

bool GEHDF5Wrapper::check_GE_signature(const std::string& filename)
{
    H5::H5File file;

    if(!file.isHdf5(filename))
        return false;

    file.openFile( filename, H5F_ACC_RDONLY );
    H5::StrType  vlst(0,37);  // 37 here is the length of the string (I got it from the text file generated by list2txt with the LIST000_decomp.BLF

    std::string read_str_scanner;
    std::string read_str_manufacturer;

    H5::DataSet dataset = file.openDataSet("/HeaderData/ExamData/scannerDesc");
    dataset.read(read_str_scanner,vlst);

    H5::DataSet dataset2= file.openDataSet("/HeaderData/ExamData/manufacturer");
    dataset2.read(read_str_manufacturer,vlst);
//PW TODO Remove check on SIGNA PET/MR.
    if(read_str_scanner == "SIGNA PET/MR" &&
            read_str_manufacturer == "GE MEDICAL SYSTEMS")
        return true;

    return false;
}

GEHDF5Wrapper::GEHDF5Wrapper()
{
    // Not much.
}

GEHDF5Wrapper::GEHDF5Wrapper(const std::string& filename)
{
    if(!file.isHdf5(filename))
        error("GEHDF5Wrapper: The input file is not HDF5! Abort.");

    if(open(filename) == Succeeded::no)
        error("GEHDF5Wrapper: Error opening HDF5 file. Abort.");
}

Succeeded
GEHDF5Wrapper::open(const std::string& filename)
{
    if(!file.isHdf5(filename))
        error("GEHDF5Wrapper: The input file is not HDF5! Abort.");

    file.openFile(filename, H5F_ACC_RDONLY);

    initialise_exam_info();

    if(GEHDF5Wrapper::check_GE_signature(filename))
    {
        warning("CListModeDataGESigna: "
                "Probably this is GESigna, but couldn't find scan start time etc."
                "The scanner will be initialised from STIR as opposed to the HDF5 header.");
        is_signa = true;

        this->scanner_sptr.reset(new Scanner(Scanner::PETMR_Signa));
        return Succeeded::yes;
    }
    else
    {
        // Read from HDF5 header ...
        return initialise_scanner_from_HDF5();
    }
}

Succeeded GEHDF5Wrapper::initialise_scanner_from_HDF5()
{
    std::string read_str_scanner;
    H5::StrType  vlst(0,37);  // 37 here is the length of the string (I got it from the text file generated by list2txt with the LIST000_decomp.BLF

    H5::DataSet dataset = file.openDataSet("/HeaderData/ExamData/scannerDesc");
    dataset.read(read_str_scanner,vlst);

    int num_transaxial_blocks_per_bucket = 0;
    int num_axial_blocks_per_bucket = 0;
    int axial_blocks_per_unit = 0;
    int radial_blocks_per_unit = 0;
    int axial_units_per_module = 0;
    int radial_units_per_module = 0;
    int axial_modules_per_system = 0;
    int radial_modules_per_system = 0;
    int max_num_non_arccorrected_bins = 0;
    int num_transaxial_crystals_per_block = 0;
    int num_axial_crystals_per_block = 0 ;
    float inner_ring_diameter = 0.0;
    float detector_axial_size = 0.0;
    float intrinsic_tilt = 0.0;
    float physical_scanner_radius = 0.0;
    int num_detector_layers = 1;
    float energy_resolution = -1.0f;
    float reference_energy = -1.0f;

    H5::DataSet str_inner_ring_diameter = file.openDataSet("/HeaderData/SystemGeometry/effectiveRingDiameter");
    H5::DataSet str_axial_blocks_per_module = file.openDataSet("/HeaderData/SystemGeometry/axialBlocksPerModule");
    H5::DataSet str_radial_blocks_per_module = file.openDataSet("/HeaderData/SystemGeometry/radialBlocksPerModule");
    H5::DataSet str_axial_blocks_per_unit = file.openDataSet("/HeaderData/SystemGeometry/axialBlocksPerUnit");
    H5::DataSet str_radial_blocks_per_unit = file.openDataSet("/HeaderData/SystemGeometry/radialBlocksPerUnit");
    H5::DataSet str_axial_units_per_module = file.openDataSet("/HeaderData/SystemGeometry/axialUnitsPerModule");
    H5::DataSet str_radial_units_per_module = file.openDataSet("/HeaderData/SystemGeometry/radialUnitsPerModule");
    H5::DataSet str_axial_modules_per_system = file.openDataSet("/HeaderData/SystemGeometry/axialModulesPerSystem");
    H5::DataSet str_radial_modules_per_system = file.openDataSet("/HeaderData/SystemGeometry/radialModulesPerSystem");
    //! \todo P.W: Find the crystal gaps and other info missing.
    H5::DataSet str_detector_axial_size = file.openDataSet("/HeaderData/SystemGeometry/detectorAxialSize");
    H5::DataSet str_intrinsic_tilt = file.openDataSet("/HeaderData/SystemGeometry/transaxial_crystal_0_offset");
    H5::DataSet str_max_number_of_non_arc_corrected_bins = file.openDataSet("/HeaderData/Sorter/dimension1Size");
    H5::DataSet str_axial_crystals_per_block = file.openDataSet("/HeaderData/SystemGeometry/axialCrystalsPerBlock");
    H5::DataSet str_radial_crystals_per_block = file.openDataSet("/HeaderData/SystemGeometry/radialCrystalsPerBlock");
    H5::DataSet str_physical_scanner_radius = file.openDataSet("/HeaderData/SystemGeometry/sourceRadius");
    //! \todo Convert to numbers.

    str_radial_blocks_per_module.read(&num_transaxial_blocks_per_bucket, H5::PredType::NATIVE_UINT32);
    str_axial_blocks_per_module.read(&num_axial_blocks_per_bucket, H5::PredType::NATIVE_UINT32);
    str_axial_blocks_per_unit.read(&axial_blocks_per_unit, H5::PredType::NATIVE_UINT32);
    str_radial_blocks_per_unit.read(&radial_blocks_per_unit, H5::PredType::NATIVE_UINT32);
    str_axial_units_per_module.read(&axial_units_per_module, H5::PredType::NATIVE_UINT32);
    str_radial_units_per_module.read(&radial_units_per_module, H5::PredType::NATIVE_UINT32);
    str_axial_modules_per_system.read(&axial_modules_per_system, H5::PredType::NATIVE_UINT32);
    str_radial_modules_per_system.read(&radial_modules_per_system, H5::PredType::NATIVE_UINT32);
    str_inner_ring_diameter.read(&inner_ring_diameter, H5::PredType::NATIVE_FLOAT);
    str_detector_axial_size.read(&detector_axial_size, H5::PredType::NATIVE_FLOAT);
    str_intrinsic_tilt.read(&intrinsic_tilt, H5::PredType::NATIVE_FLOAT);
    str_max_number_of_non_arc_corrected_bins.read(&max_num_non_arccorrected_bins, H5::PredType::NATIVE_UINT32);
    str_radial_crystals_per_block.read(&num_transaxial_crystals_per_block, H5::PredType::NATIVE_UINT32);
    str_axial_crystals_per_block.read(&num_axial_crystals_per_block, H5::PredType::NATIVE_UINT32);
    str_physical_scanner_radius.read(&physical_scanner_radius, H5::PredType::NATIVE_FLOAT);

    int num_rings  = num_axial_blocks_per_bucket*num_axial_crystals_per_block*axial_modules_per_system;
    int num_detectors_per_ring = num_transaxial_blocks_per_bucket*num_transaxial_crystals_per_block*radial_modules_per_system;
    int default_num_arccorrected_bins = max_num_non_arccorrected_bins;
    float inner_ring_radius = physical_scanner_radius;
    float average_depth_of_interaction = 0.5f*inner_ring_diameter-physical_scanner_radius; // Assuming this to be constant. Although this will change depending on scanner.
    float ring_spacing = detector_axial_size/num_rings;

    //! \todo : bin_size
    float bin_size = static_cast<float>(max_num_non_arccorrected_bins)/inner_ring_radius;
    int num_axial_crystals_per_singles_unit = 1;
    int num_transaxial_crystals_per_singles_unit = 1;

    //PW Not sure what to put for scanner here.
    //PW TODO Change the User_defined_scanner to respective scanner name.
    //PW TODO Still to set the default bin size, maximum number of non-arc-corrected bins and default number of arc-corrected bins.
    this->scanner_sptr.reset(new Scanner(Scanner::User_defined_scanner,
                                         read_str_scanner, num_detectors_per_ring,
                                         num_rings,
                                         max_num_non_arccorrected_bins,
                                         default_num_arccorrected_bins,
                                         inner_ring_diameter/2,
                                         average_depth_of_interaction,
                                         ring_spacing,
                                         bin_size,
                                         intrinsic_tilt*_PI/180,
                                         num_axial_blocks_per_bucket,
                                         num_transaxial_blocks_per_bucket,
                                         num_axial_crystals_per_block,
                                         num_transaxial_crystals_per_block,
                                         num_axial_crystals_per_singles_unit,
                                         num_transaxial_crystals_per_singles_unit,
                                         num_detector_layers,
                                         energy_resolution,
                                         reference_energy));

    return Succeeded::yes;
}

Succeeded GEHDF5Wrapper::initialise_exam_info()
{
    this->exam_info_sptr.reset(new ExamInfo());

    // PW Get the high and low energy threshold values from HDF5 header.
    unsigned int low_energy_thres = 0;
    unsigned int high_energy_thres = 0;

    H5::DataSet str_low_energy_thres = file.openDataSet("/HeaderData/AcqParameters/EDCATParameters/lower_energy_limit");
    H5::DataSet str_high_energy_thres = file.openDataSet("/HeaderData/AcqParameters/EDCATParameters/upper_energy_limit");

    str_low_energy_thres.read(&low_energy_thres, H5::PredType::NATIVE_UINT32);
    str_high_energy_thres.read(&high_energy_thres, H5::PredType::NATIVE_UINT32);

    // PW Set these values in exam_info_sptr.
    exam_info_sptr->set_high_energy_thres(static_cast<float>(low_energy_thres));
    exam_info_sptr->set_low_energy_thres(static_cast<float>(high_energy_thres));


    //! \todo convert time slices to timeFrameDefinitions
    //NE Copied from SignesRatesFromGEHDF5:
    //PW Get the total number of time slices from the HDF5 file format.

    unsigned int num_time_slices = 0;
    H5::DataSet timeframe_dataspace = file.openDataSet("/HeaderData/SinglesHeader/numValidSamples");
    timeframe_dataspace.read(&num_time_slices, H5::PredType::NATIVE_UINT32);
    std::vector<std::pair<double, double> >tf(num_time_slices);

    for (int i = 0; i < num_time_slices; ++i)
    {
        tf[i].first = i;
        tf[i].second = i + 1;
    }

    TimeFrameDefinitions tm(tf);
    exam_info_sptr->set_time_frame_definitions(tm);

    return Succeeded::yes;
}

Succeeded GEHDF5Wrapper::initialise_listmode_data(const std::string &path)
{
    if(path.size() == 0)
    {
        if(is_signa)
        {
            m_address = "/ListData/listData";
            //! \todo Get these numbers from the HDF5 file
            {
            m_size_of_record_signature = 6;
            m_max_size_of_record = 16;
            }
        }
        else
            return Succeeded::no;
    }
    else
        m_address = path;

    m_dataset_sptr.reset(new H5::DataSet(file.openDataSet(m_address)));

    m_dataspace = m_dataset_sptr->getSpace();
    int dataset_list_Ndims = m_dataspace.getSimpleExtentNdims();

    hsize_t dims_out[dataset_list_Ndims];
    m_dataspace.getSimpleExtentDims( dims_out, NULL);
    m_list_size=dims_out[0];
    const long long unsigned int tmp_size_of_record_signature = m_size_of_record_signature;
    m_memspace_ptr = new H5::DataSpace( dataset_list_Ndims,
                            &tmp_size_of_record_signature);


    return Succeeded::yes;
}

Succeeded GEHDF5Wrapper::initialise_singles_data(const std::string &path)
{
    if(path.size() == 0)
    {
        if(is_signa)
        {
            m_address = "/Singles/CrystalSingles/sample";
            //! \todo Get these numbers from the HDF5 file
            {
                m_NX_SUB = 45;    // hyperslab dimensions
                m_NY_SUB = 448;
                m_NZ_SUB = 1;
                m_NX = 45;        // output buffer dimensions
                m_NY = 448;
                m_NZ = 1;
            }
        }
        else
            return Succeeded::no;
    }
    else
        m_address = path;

    return Succeeded::yes;
}

Succeeded GEHDF5Wrapper::initialise_proj_data_data(const std::string& path,
                                                 const unsigned int view_num)
{
    if(path.size() == 0)
    {
        if(is_signa)
        {
            m_address = "/SegmentData/Segment2/3D_TOF_Sinogram/view";
            if(view_num > 0)
            {
                std::ostringstream datasetname;
                datasetname << m_address << view_num;
                m_dataset_sptr.reset(new H5::DataSet(file.openDataSet(datasetname.str())));
                m_dataspace = m_dataset_sptr->getSpace();

//                m_memspace_ptr = new H5::DataSpace()
            }
            //! \todo Get these numbers from the HDF5 file
            {
                m_NX_SUB = 1981;    // hyperslab dimensions
                m_NY_SUB = 27;
                m_NZ_SUB = 357;
                m_NX = 45;        // output buffer dimensions
                m_NY = 448;
                m_NZ = 357;
            }
        }
        else
            return Succeeded::no;
    }
    else
        m_address = path;

    return Succeeded::yes;
}

// PW The geo factors are stored in geo3d file under the file path called /SegmentData/Segment4/3D_Norm_correction/slice%d where
// slice numbers go from 1 to 16. Here this path is initialised, along with the output buffer and hyperslab.
//
Succeeded GEHDF5Wrapper::initialise_geo_factors_data(const std::string& path,
                                                 const unsigned int slice_num)
{
    if(path.size() == 0)
    {
        if(is_signa)
        {
            m_address = "/SegmentData/Segment4/3D_Norm_Correction/slice";
            if(slice_num > 0)
            {
                std::ostringstream datasetname;
                datasetname << m_address << slice_num;
                m_dataset_sptr.reset(new H5::DataSet(file.openDataSet(datasetname.str())));
                m_dataspace = m_dataset_sptr->getSpace();
                // PW here I output the dataspace dimensions and order to be correctly translated in the main code.
                int rank = m_dataspace.getSimpleExtentNdims();
                hsize_t dims_out[2];
          //      int ndims = m_dataspace.getSimpleExtentDims( dims_out, NULL);
                     std::cout << "rank " << rank << ", dimensions " <<
                         (unsigned long)(dims_out[0]) << " x " <<
                         (unsigned long)(dims_out[1]) << std::endl;
//                m_memspace_ptr = new H5::DataSpace()
            }
            //! \todo Get these numbers from the HDF5 file
            {
                m_NX_SUB = 1981;    // hyperslab dimensions
                m_NY_SUB = 357;
                m_NX = 1981;        // output buffer dimensions
                m_NY = 357;
            }
        }
        else
            return Succeeded::no;
    }
    else
        m_address = path;

    return Succeeded::yes;
}

Succeeded GEHDF5Wrapper::initialise_efficiency_factors(const std::string& path)
{
    if(path.size() == 0)
    {
        if(is_signa)
        {
            m_address = "3DCrystalEfficiency/crystalEfficiency";
            m_dataset_sptr.reset(new H5::DataSet(file.openDataSet(m_address)));
            m_dataspace = m_dataset_sptr->getSpace();
            int rank = m_dataspace.getSimpleExtentNdims();
            hsize_t dims_out[2];
           // int ndims = m_dataspace.getSimpleExtentDims( dims_out, NULL);
                 std::cout << "rank " << rank << ", dimensions " <<
                     (unsigned long)(dims_out[0]) << " x " <<
                     (unsigned long)(dims_out[1]) << std::endl;
            //! \todo Get these numbers from the HDF5 file
            {
                m_NX_SUB = 45;    // hyperslab dimensions
                m_NY_SUB = 448;
                m_NZ_SUB = 1;
                m_NX = 45;        // output buffer dimensions
                m_NY = 448;
                m_NZ = 1;
            }
        }
        else
            return Succeeded::no;
    }
    else
        m_address = path;

    return Succeeded::yes;
}

// Developed for listmode access
Succeeded GEHDF5Wrapper::get_from_dataspace(std::streampos& current_offset, char* output)
{
    hsize_t pos = static_cast<hsize_t>(current_offset);
    m_dataspace.selectHyperslab( H5S_SELECT_SET, &m_size_of_record_signature, &pos );
    m_dataset_sptr->read( output, H5::PredType::STD_U8LE, *m_memspace_ptr, m_dataspace );
    current_offset += static_cast<std::streampos>(m_size_of_record_signature);

    //  // TODO error checking
    return Succeeded::yes;
}

// Developed for ProjData
Succeeded GEHDF5Wrapper::get_from_dataset(const std::array<unsigned long long int, 3>& offset,
                                        const std::array<unsigned long long int, 3>& count,
                                        const std::array<unsigned long long int, 3>& stride,
                                        const std::array<unsigned long long int, 3>& block,
                                        Array<1, unsigned char> &output)
{
    m_dataspace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data());
    m_memspace_ptr= new H5::DataSpace(3, count.data());
    m_dataset_sptr->read(output.get_data_ptr(), H5::PredType::STD_U8LE, *m_memspace_ptr, m_dataspace);
    output.release_data_ptr();

    //  // TODO error checking
    return Succeeded::yes;
}

//! \todo Array read as UINT32 should have an output of std::uint32_t.
//! \todo Check read data type as it's unlikely that it is NATIVE_UINT32, it may be STD_U32LE.
//PW Developed for Geometric Correction Factors
Succeeded GEHDF5Wrapper::get_from_2d_dataset(const std::array<unsigned long long int, 2>& offset,
                                        const std::array<unsigned long long int, 2>& count,
                                        const std::array<unsigned long long int, 2>& stride,
                                        const std::array<unsigned long long int, 2>& block,
                                        Array<1, unsigned int> &output)
{
    m_dataspace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data());
    m_memspace_ptr= new H5::DataSpace(2, count.data());
    m_dataset_sptr->read(output.get_data_ptr(), H5::PredType::NATIVE_UINT32, *m_memspace_ptr, m_dataspace);
    output.release_data_ptr();

    //  // TODO error checking
    return Succeeded::yes;
}

//PW Developed for Efficiency Factors
Succeeded GEHDF5Wrapper::get_from_2d_dataset(const std::array<unsigned long long int, 2>& offset,
                                        const std::array<unsigned long long int, 2>& count,
                                        const std::array<unsigned long long int, 2>& stride,
                                        const std::array<unsigned long long int, 2>& block,
                                        Array<1, float> &output)
{
    m_dataspace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data());
    m_memspace_ptr= new H5::DataSpace(2, count.data());
    m_dataset_sptr->read(output.get_data_ptr(), H5::PredType::NATIVE_FLOAT, *m_memspace_ptr, m_dataspace);
    output.release_data_ptr();

    //  // TODO error checking
    return Succeeded::yes;
}

//! \todo Array read as UINT32 should have an output of std::uint32_t.
//! \todo Check the H5 data type as it's unlikely that it is NATIVE_UINT32, it may be STD_U32LE.
// Developed for Singles
Succeeded GEHDF5Wrapper::get_dataspace(const unsigned int current_id,
                                     Array<1, unsigned int>& output)
{
    std::ostringstream datasetname;
    datasetname << "/Singles/CrystalSingles/sample" << current_id;
    m_dataset_sptr.reset(new H5::DataSet(file.openDataSet(datasetname.str())));
    m_dataset_sptr->read(output.get_data_ptr(), H5::PredType::NATIVE_UINT32);
    output.release_data_ptr();

    //  // TODO error checking
    return Succeeded::yes;
}

//! \todo Array read as UINT32 should have an output of std::uint32_t.
//! \todo Check the H5 data type as it's unlikely that it is NATIVE_UINT32, it may be STD_U32LE.
// Developed for Singles
Succeeded GEHDF5Wrapper::get_dataspace(const unsigned int current_id,
                                     Array<2, unsigned int>& output)
{
    std::ostringstream datasetname;
    datasetname << "/Singles/CrystalSingles/sample" << current_id;
    m_dataset_sptr.reset(new H5::DataSet(file.openDataSet(datasetname.str())));
    m_dataset_sptr->read( output[current_id].get_data_ptr(), H5::PredType::NATIVE_UINT32);
    output[current_id].release_data_ptr();

    //  // TODO error checking
    return Succeeded::yes;
}

} // namespace
}
END_NAMESPACE_STIR

