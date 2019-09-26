#pragma once
#include<iostream>
#include <fstream>
#include <exception>
#include <string>
#include <array>
#include "mat.h"
#include <algorithm>
#include <random>

#define num_cluster 2
using vec3b = std::array<int, 3>;
using vec2b = std::array<int, 2>;
using vec4b = std::array<int, 4>;
using vec6b = std::array<int, 6>;


using namespace std;
class Clustering
{
  public:
	static uint8_t get_byte(std::istream& is)
	{
		int val;
		is >> val;
		if (0 <= val && val <= 255)
			return val;
		else
			throw std::range_error("Wrong pixel value");
	}

	bool load_ppm(const std::string& filename, mat<vec3b>& img, mat<vec6b>& matrix, vector<vec4b>& centroid)
	{
		using namespace std;
		ifstream is(filename, ios::binary);
		if (!is)
			return false;

		string MagicNumber;
		is >> MagicNumber;
		bool ascii;
		if (MagicNumber == "P3")
			ascii = true;
		else if (MagicNumber == "P6")
			ascii = false;
		else
			return false;

		if (is.get() != '\n')
			return false;

		if (is.peek() == '#') {
			string comment;
			getline(is, comment);
		}

		int L, A, MaxVal;
		is >> L;
		if (is.get() != ' ')
			return false;
		is >> A;
		if (is.get() != '\n')
			return false;
		is >> MaxVal;
		if (MaxVal != 255 || is.get() != '\n')
			return false;

		img.resize(A, L);
		matrix.resize(A, L);

		int count_num_cluster = 0;
		if (ascii) {
			uint8_t red, green, blue;
			for (int r = 0; r < img.rows(); ++r) {
				for (int c = 0; c < img.cols(); ++c) {
					try {

						red = get_byte(is);
						green = get_byte(is);
						blue = get_byte(is);
						img(r, c)[0] = red;
						img(r, c)[1] = green;
						img(r, c)[2] = blue;
						matrix(r, c)[0] = static_cast<int>(red);
						matrix(r, c)[1] = static_cast<int>(green);
						matrix(r, c)[2] = static_cast<int>(blue);
						matrix(r, c)[3] = r;
						matrix(r, c)[4] = c;
						if(count_num_cluster < num_cluster)
						{
							vec4b elem;
							elem[0] = red;
							elem[1] = green;
							elem[2] = blue;		
							if (count_num_cluster != 0)
							{
								if(centroid[count_num_cluster -1][0] != red || centroid[count_num_cluster -1][1] != green || centroid[count_num_cluster -1][2] != blue)
								{
									centroid.push_back(elem);
									count_num_cluster++;
								}
							}
							else
							{
								centroid.push_back(elem);
								count_num_cluster++;
							}
						}
					}
					catch (const exception& e) {
						std::cout << e.what() << "\n";
						return false;
					};
				}
			}
		}
		else {
			is.read(img.rawdata(), img.rawsize());

			char* val = img.rawdata();
			cout << "the val is " << val;		
		}		
		return true;
	}

	double distance(vec4b first, vec4b second)
	{
		return sqrt(pow(first[0] - second[0], 2) + pow(first[1] - second[1], 2) + pow(first[2] - second[2], 2));		
	}

	void initialize_centroid(vector<vec4b>& centroid, mat<vec6b>& matrix, vector<vec4b>& median)
	{
		std::random_device rand;

		for (size_t index = 0; index < num_cluster; index++)
		{
			centroid[index][0] = rand()% 255;
			centroid[index][1] = rand() % 255;
			centroid[index][2] = rand() % 255;
			centroid[index][3] = 0;
			median.push_back({0, 0, 0, 0});			
		}	

		for (size_t r = 0; r < matrix.rows(); r++)
		{
			double distances[num_cluster];

			for (size_t c = 0; c < matrix.cols(); c++)
			{
				vec4b elem = { matrix(r, c)[0], matrix(r, c)[1] ,  matrix(r, c)[2] };

				for (size_t index = 0; index < num_cluster; index++)
				{
					distances[index] = distance(elem, centroid[index]);
				}

				matrix(r, c)[5] = min_element(distances, distances + num_cluster) - distances;				
			}
		}

		int count_zero = 0, count_uno = 0;

		for (size_t r = 0; r < matrix.rows(); r++)
		{
			double distances[num_cluster];

			for (size_t c = 0; c < matrix.cols(); c++)
			{
				if (matrix(r, c)[5] == 0)
				{
					count_zero++;
				}
				else if (matrix(r, c)[5] == 1)
				{
					count_uno++;
				}

			}
		}

		cout << " count_zero " << count_zero << "count_uno  " << count_uno << endl;
	}
	

	
	void cluster(vector<vec4b>& centroid, mat<vec6b>& matrix, vector<vec4b>& median)
	{
		int clustered = false;	

		int count = 0;
		while (!clustered)
		{
			count++;
			if (count == 10)
			{
				break;
			}
			clustered = true;	

			for (size_t r = 0; r < matrix.rows(); r++)
			{
				double distances[num_cluster];

				for (size_t c = 0; c < matrix.cols(); c++)
				{
					vec4b elem = {matrix(r, c)[0], matrix(r, c)[1] ,  matrix(r, c)[2]};					
					
					for (size_t index = 0; index < num_cluster; index++)
					{						
						distances[index] = distance(elem, centroid[index]);
					}										

					if ((min_element(distances, distances + num_cluster) - distances) != matrix(r, c)[5])
					{						
						matrix(r, c)[5] = min_element(distances, distances + num_cluster) - distances;						
						clustered = false;
					}

					median[matrix(r, c)[5]][0] += matrix(r, c)[1];
					median[matrix(r, c)[5]][1] += matrix(r, c)[2];
					median[matrix(r, c)[5]][2] += matrix(r, c)[3];
					median[matrix(r, c)[5]][3]++;
				}				
			}	

			for (size_t index = 0; index < num_cluster; index++)
			{
				centroid[index][0] = median[index][0] / median[index][3];
				centroid[index][1] = median[index][1] / median[index][3];
				centroid[index][2] = median[index][2] / median[index][3];
				centroid[index][3] = median[index][3];
			}
		}
	}

	bool save_image(mat<vec6b> img)
	{
		std::ofstream os("output.ppm", std::ios::binary);
		if (!os)
			return false;

		os << "P3" << "\n";
		os << "# EdDM\n";
		os << img.cols() << " " << img.rows() << "\n";
		os << "255\n";


		for (int r = 0; r < img.rows(); ++r) {
			for (int c = 0; c < img.cols(); ++c) {
				if (img(r, c)[5] == 0)
				{
					os << static_cast<int>(img(r, c)[0]) << " "
						<< static_cast<int>(img(r, c)[1]) << " "
						<< static_cast<int>(img(r, c)[2]) << " ";
				}
				else
				{
					os << 0 << " "
						<< 0 << " "
						<< 0 << " ";
				}

			}
			os << "\n";
		}
	}
};

int main()
{
	vector<vec4b> centroid;
	vector<vec4b> median;
	mat<vec6b> matrix;
	mat<vec3b> img;
	Clustering clt;
	
	clt.load_ppm("flip.ppm", img, matrix, centroid);
	clt.initialize_centroid(centroid, matrix, median);


	clt.cluster(centroid, matrix, median);
	clt.save_image(matrix);

	cout << endl;


	int count_zero = 0, count_uno = 0;

	for (size_t r = 0; r < matrix.rows(); r++)
	{
		double distances[num_cluster];

		for (size_t c = 0; c < matrix.cols(); c++)
		{
			if (matrix(r, c)[5] == 0)
			{
				count_zero++;
			}
			else if (matrix(r, c)[5] == 1)
			{
				count_uno++;
			}

		}
	}

	cout << " count_zero " << count_zero << "count_uno  " << count_uno << endl;
	//// using default comparison:
	//std::cout << "The smallest element is " << *std::min_element(v.begin(), v.end()) << '\n';
	//std::cout << "The smallest index is " << std::min_element(v.begin(), v.end()) - v.begin() << '\n';
}