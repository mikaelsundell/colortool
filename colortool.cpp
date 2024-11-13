//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - present Mikael Sundell.
//

#include <iostream>
#include <fstream>
#include <algorithm>

// openimageio
#include <OpenImageIO/argparse.h>
#include <OpenImageIO/filesystem.h>
#include <OpenImageIO/sysutil.h>

using namespace OIIO;

// eigen
#include <Eigen/Dense>

// boost
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace boost::property_tree;

// prints
void
print_precision(int precision) {
    std::cout << std::fixed << std::setprecision(precision);
}

template <typename T>
static void
print_info(std::string param, const T& value = T()) {
    std::cout << "info: " << param << value << std::endl;
}

static void
print_info(std::string param) {
    print_info<std::string>(param);
}

template <typename Derived>
static void print_eigen(const std::string& param, const Eigen::MatrixBase<Derived>& value) {
    std::cout << "info: " << param;
    if (value.cols() == 1 || value.rows() == 1) {
        for (int i = 0; i < value.size(); ++i) {
            std::cout << value(i);
            if (i < value.size() - 1) {
                std::cout << ", ";
            }
        }
    } else {
        std::cout << std::endl << "info:     ";
        for (int i = 0; i < value.rows(); ++i) {
            for (int j = 0; j < value.cols(); ++j) {
                std::cout << value(i, j);
                if (j < value.cols() - 1) {
                    std::cout << ", ";
                }
            }
            std::cout << std::endl;
            if (i < value.rows() - 1) {
                std::cout << "info:     ";
            }
        }
    }
    std::cout << std::endl;
}

template <typename T>
static void
print_warning(std::string param, const T& value = T()) {
    std::cout << "warning: " << param << value << std::endl;
}

static void
print_warning(std::string param) {
    print_warning<std::string>(param);
}

template <typename T>
static void
print_error(std::string param, const T& value = T()) {
    std::cerr << "error: " << param << value << std::endl;
}

static void
print_error(std::string param) {
    print_error<std::string>(param);
}

// adaptation methods
enum
AdaptationMethod {
    None,
    XYZScaling,
    Bradford,
    VonKries
};

// color tool
struct ColorTool
{
    bool help = false;
    bool verbose = false;
    bool colorspaces = false;
    AdaptationMethod adaptationmethod = Bradford;
    std::string inputcolorspace;
    std::string outputcolorspace;
    int code = EXIT_SUCCESS;
};

static ColorTool tool;

static int
set_adaptationmethod(int argc, const char* argv[])
{
    OIIO_DASSERT(argc == 2);
    std::string str(argv[1]);
    if (str == "xyzscaling") {
        tool.adaptationmethod = AdaptationMethod::XYZScaling;
    }
    else if (str == "bradford") {
        tool.adaptationmethod = AdaptationMethod::Bradford;
    }
    else if (str == "vonkries") {
        tool.adaptationmethod = AdaptationMethod::VonKries;
    }
    else {
        print_error("could not parse adaptation method: ", str);
        return 1;
    }
    return 0;
}

static void
print_help(ArgParse& ap)
{
    ap.print_help();
}

// utils - colorspaces
Eigen::Vector3d xy_to_xyz(const Eigen::Vector2d& xy)
{
    return Eigen::Vector3d(xy.x() / xy.y(), 1.0, (1.0 - xy.x() - xy.y()) / xy.y());
}

Eigen::Matrix3d rgb_to_xyz(const Eigen::Vector3d& r, const Eigen::Vector3d& g, const Eigen::Vector3d& b, const Eigen::Vector3d& whitepoint)
{
    Eigen::Matrix3d m; // matrix from primaries
    m.col(0) = r;
    m.col(1) = g;
    m.col(2) = b;
    Eigen::Vector3d s = m.inverse() * whitepoint; // scaling factors S using whitepoint
    Eigen::Matrix3d xyz;
    xyz.col(0) = s(0) * m.col(0);
    xyz.col(1) = s(1) * m.col(1);
    xyz.col(2) = s(2) * m.col(2);
    return xyz;
}

Eigen::Matrix3d adaptation_matrix(AdaptationMethod method) {
    Eigen::Matrix3d m;
    if (method == XYZScaling) {
        m <<  1.0, 0.0, 0.0,
              0.0, 1.0, 0.0,
              0.0, 0.0, 1.0;
    }
    else if (method == Bradford) {
        m <<  0.8951,  0.2664, -0.1614,
             -0.7502,  1.7135,  0.0367,
              0.0389, -0.0685,  1.0296;
    }
    else if (method == VonKries) {
        m <<  0.40024,  0.70760, -0.08081,
             -0.22630,  1.16532,  0.04570,
              0.00000,  0.00000,  0.91822;
    }
    return m;
}

Eigen::Matrix3d adaptation_matrix(const Eigen::Vector3d& source, const Eigen::Vector3d& target, AdaptationMethod method) {
    Eigen::Matrix3d m = adaptation_matrix(method);
    Eigen::Vector3d sourcelms = m * source;
    Eigen::Vector3d targetlms = m * target;
    Eigen::Matrix3d scale = targetlms.cwiseQuotient(sourcelms).asDiagonal(); // compute scaling factors
    Eigen::Matrix3d adaptationMatrix = m.inverse() * scale * m; // computer final adaptation
    return adaptationMatrix;
}

// utils - filesystem
std::string program_path(const std::string& path)
{
    return Filesystem::parent_path(Sysutil::this_program_path()) + path;
}

std::string resources_path(const std::string& resource)
{
    return Filesystem::parent_path(Sysutil::this_program_path()) + "/resources/" + resource;
}

// colorspace
struct Colorspace
{
    std::string name;
    Eigen::Vector2d r;
    Eigen::Vector2d g;
    Eigen::Vector2d b;
    Eigen::Vector2d whitepoint;
};

// main
int
main( int argc, const char * argv[])
{
    // Helpful for debugging to make sure that any crashes dump a stack
    // trace.
    Sysutil::setup_crash_stacktrace("stdout");

    Filesystem::convert_native_arguments(argc, (const char**)argv);
    ArgParse ap;

    ap.intro("colortool -- a utility set for color space conversions, with support for white point adaptation\n");
    ap.usage("colortool [options] filename...")
      .add_help(false)
      .exit_on_error(true);
    
    ap.separator("General flags:");
    ap.arg("--help", &tool.help)
      .help("Print help message");
    
    ap.arg("-v", &tool.verbose)
      .help("Verbose status messages");
    
    ap.arg("--colorspaces", &tool.colorspaces)
      .help("List all colorspaces");
    
    ap.arg("--adaptationmethod %s:adaptationmethod")
      .help("Adaptation methods: xyzscaling, bradford, vonkries, default: bradford")
      .action(set_adaptationmethod);

    ap.separator("Input flags:");
    ap.arg("--inputcolorspace %s:FILE", &tool.inputcolorspace)
      .help("Input color space");
    
    ap.separator("Output flags:");
    ap.arg("--outputcolorspace %s:FILE", &tool.outputcolorspace)
      .help("Output color space");

    // clang-format on
    if (ap.parse_args(argc, (const char**)argv) < 0) {
        print_error("Could no parse arguments: ", ap.geterror());
        print_help(ap);
        ap.abort();
        return EXIT_FAILURE;
    }
    if (ap["help"].get<int>()) {
        print_help(ap);
        ap.abort();
        return EXIT_SUCCESS;
    }
    
    if (!tool.colorspaces) {
        if (!tool.inputcolorspace.size()) {
            print_error("missing parameter: ", "inputcolorspace");
            ap.briefusage();
            ap.abort();
            return EXIT_FAILURE;
        }
        if (!tool.outputcolorspace.size()) {
            print_error("missing parameter: ", "outputcolorspace");
            ap.briefusage();
            ap.abort();
            return EXIT_FAILURE;
        }
        if (argc <= 1) {
            ap.briefusage();
            print_error("For detailed help: colortool --help");
            return EXIT_FAILURE;
        }
    }
    
    // colortool program
    print_info("colortool -- a utility set for color space conversions, with support for white point adaptation.");
    
    // precision
    print_precision(6);

    // colorspaces
    std::map<std::string, Colorspace> colorspaces;
    std::string jsonfile = resources_path("colorspaces.json");
    std::ifstream json(jsonfile);
    if (json.is_open()) {
        ptree pt;
        read_json(jsonfile, pt);
        for (const std::pair<const ptree::key_type, ptree&>& item : pt) {
            std::string name = item.first;
            const ptree& data = item.second;
            Colorspace cs;
            cs.name = name;
            bool valid = true;
            try {
                cs.r.x() = data.get<double>("primaries.R.x", 0.0f);
                cs.r.y() = data.get<double>("primaries.R.y", 0.0f);
                cs.g.x() = data.get<double>("primaries.G.x", 0.0f);
                cs.g.y() = data.get<double>("primaries.G.y", 0.0f);
                cs.b.x() = data.get<double>("primaries.B.x", 0.0f);
                cs.b.y() = data.get<double>("primaries.B.y", 0.0f);
                cs.whitepoint.x() = data.get<double>("whitepoint.x", 0.0f);
                cs.whitepoint.y() = data.get<double>("whitepoint.y", 0.0f);
            } catch (const boost::property_tree::ptree_error& e) {
                valid = false;
                print_error("missing or invalid value in colorspace: ", e.what());
                return EXIT_FAILURE;
            }
            colorspaces[name] = cs;
        }
    } else {
        print_error("could not open colorspace file: ", jsonfile);
        ap.abort();
        return EXIT_FAILURE;
    }
    
    if (tool.colorspaces) {
        print_info("Colorspaces:");
        for (const std::pair<std::string, Colorspace>& pair : colorspaces) {
            print_info("    ", pair.first);
        }
        return EXIT_SUCCESS;
    }
    
    if (!colorspaces.count(tool.inputcolorspace)) {
        print_error("unknown input colorsoace: ", tool.inputcolorspace);
        ap.abort();
        return EXIT_FAILURE;
    }
    
    if (!colorspaces.count(tool.outputcolorspace)) {
        print_error("unknown output colorsoace: ", tool.outputcolorspace);
        ap.abort();
        return EXIT_FAILURE;
    }

    // input colorspace
    Eigen::Matrix3d inputxyz;
    Colorspace inputcolorspace = colorspaces[tool.inputcolorspace];
    Eigen::Vector3d inputwhitepoint;
    print_info("input colorspace: ", inputcolorspace.name);
    {
        Eigen::Vector3d r = xy_to_xyz(inputcolorspace.r);
        Eigen::Vector3d g = xy_to_xyz(inputcolorspace.g);
        Eigen::Vector3d b = xy_to_xyz(inputcolorspace.b);
        inputwhitepoint = xy_to_xyz(inputcolorspace.whitepoint);
        if (tool.verbose) {
            print_info("  XY");
            print_eigen("    r: ", inputcolorspace.r);
            print_eigen("    g: ", inputcolorspace.g);
            print_eigen("    b: ", inputcolorspace.b);
            print_eigen("    whitepoint: ", inputcolorspace.whitepoint);
            print_info("  XYZ");
            print_eigen("    r: ", r);
            print_eigen("    g: ", g);
            print_eigen("    b: ", b);
            print_eigen("    whitepoint: ", inputwhitepoint);
        }
        inputxyz = rgb_to_xyz(r, g, b, inputwhitepoint);
        print_info("  RGB XYZ");
        print_eigen("    matrix: ", inputxyz);
        print_info("  XYZ RGB");
        print_eigen("    matrix: ", inputxyz.inverse());
    }
    
    // output color space
    Eigen::Matrix3d outputxyz;
    Colorspace outputcolorspace = colorspaces[tool.outputcolorspace];
    Eigen::Vector3d outputwhitepoint;
    print_info("output color space: ", outputcolorspace.name);
    {
        outputcolorspace = colorspaces[tool.outputcolorspace];
        Eigen::Vector3d r = xy_to_xyz(outputcolorspace.r);
        Eigen::Vector3d g = xy_to_xyz(outputcolorspace.g);
        Eigen::Vector3d b = xy_to_xyz(outputcolorspace.b);
        outputwhitepoint = xy_to_xyz(outputcolorspace.whitepoint);
        if (tool.verbose) {
           
            print_info("  XY");
            print_eigen("    r: ", outputcolorspace.r);
            print_eigen("    g: ", outputcolorspace.g);
            print_eigen("    b: ", outputcolorspace.b);
            print_eigen("    whitepoint: ", outputcolorspace.whitepoint);
            print_info("  XYZ");
            print_eigen("    r: ", r);
            print_eigen("    g: ", g);
            print_eigen("    b: ", b);
            print_eigen("    whitepoint: ", outputwhitepoint);
        }
        outputxyz = rgb_to_xyz(r, g, b, outputwhitepoint);
        print_info("  RGB XYZ");
        print_eigen("    matrix: ", outputxyz);
        print_info("  XYZ RGB");
        print_eigen("    matrix: ", outputxyz.inverse());
    }

    // whitepoint adaptation
    Eigen::Matrix3d adaptation;
    adaptation = adaptation_matrix(inputwhitepoint, outputwhitepoint, tool.adaptationmethod);
    std::string adaptationmethod;
    if (tool.adaptationmethod == AdaptationMethod::XYZScaling) {
        adaptationmethod = "XYZScaling";
    }
    else if (tool.adaptationmethod == AdaptationMethod::Bradford) {
        adaptationmethod = "Bradford";
    }
    else if (tool.adaptationmethod == AdaptationMethod::VonKries) {
        adaptationmethod = "VonKries";
    }
    print_info("whitepoint adaptation: ", adaptationmethod);
    print_eigen("    matrix: ", adaptation);
    if (tool.verbose) {
        print_info("input color space: ", inputcolorspace.name);
        print_eigen("    whitepoint: ", inputcolorspace.whitepoint);
        print_eigen("    whitepoint xyz: ", inputwhitepoint);
        print_info("output color space: ", outputcolorspace.name);
        print_eigen("    whitepoint: ", outputcolorspace.whitepoint);
        print_eigen("    whitepoint xyz: ", outputwhitepoint);
    }
    
    // transform
    Eigen::Matrix3d transform;
    transform = outputxyz.inverse() * adaptation * inputxyz;
    print_info("input to output transformation");
    print_eigen("    matrix: ", transform);
    return 0;
}
