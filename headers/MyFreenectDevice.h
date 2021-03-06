#ifndef _MYFREENECTDECICE_H
#define _MYFREENECTDECICE_H

#include "Mutex.h"
#include "libfreenect.hpp"
#include <cmath>

/* thanks to Yoda---- from IRC */
class MyFreenectDevice : public Freenect::FreenectDevice {
public:
	MyFreenectDevice(freenect_context *_ctx, int _index)
	: Freenect::FreenectDevice(_ctx, _index), m_buffer_depth_rgb(freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB).bytes),m_buffer_depth(freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB).bytes),m_buffer_video(freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB).bytes), m_gamma(2048), m_gamma_disp(2048), m_new_rgb_frame(false), m_new_depth_frame(false), m_new_depth_rgb_frame(false)
	{
		// initialize lookup tables
		for( unsigned int i = 0 ; i < 2048 ; i++) {
			// original code from libfreenect
			float v = i/2048.0;
			v = std::pow(v, 3)* 6;
			m_gamma[i] = v*6*256;

			// From http://vvvv.org/forum/the-kinect-thread, disparity to depth(cm)
			m_gamma_disp[i] = tan((float)i/1024.0f + 0.5f)*33.825f + 5.7f;
		}
	}

	// Do not call directly even in child
	void VideoCallback(void* _rgb, uint32_t timestamp) {
		video_timestamp = timestamp;
		Mutex::ScopedLock lock(m_rgb_mutex);
		uint8_t* rgb = static_cast<uint8_t*>(_rgb);
		std::copy(rgb, rgb+getVideoBufferSize(), m_buffer_video.begin());
		m_new_rgb_frame = true;
	};
	// Do not call directly even in child
	void DepthCallback(void* _depth, uint32_t timestamp) {
		Mutex::ScopedLock lock(m_depth_mutex);
		depth_timestamp = timestamp;
		uint16_t* depth = static_cast<uint16_t*>(_depth);

		for( unsigned int i = 0 ; i < 640*480 ; i++) {
			/*
			 * get depth from disparity value using precomputed lookup table. 
			 */
			m_buffer_depth[i] = m_gamma_disp[depth[i]];
	
			/*
			 * fancy bit shifting to generate the colored image mapping different
			 * depths to different colors
			 */
			int pval = m_gamma[depth[i]];
			int lb = pval & 0xff;
			switch (pval>>8) {
				case 0:
					m_buffer_depth_rgb[3*i+0] = 255;
					m_buffer_depth_rgb[3*i+1] = 255-lb;
					m_buffer_depth_rgb[3*i+2] = 255-lb;
					break;
				case 1:
					m_buffer_depth_rgb[3*i+0] = 255;
					m_buffer_depth_rgb[3*i+1] = lb;
					m_buffer_depth_rgb[3*i+2] = 0;
					break;
				case 2:
					m_buffer_depth_rgb[3*i+0] = 255-lb;
					m_buffer_depth_rgb[3*i+1] = 255;
					m_buffer_depth_rgb[3*i+2] = 0;
					break;
				case 3:
					m_buffer_depth_rgb[3*i+0] = 0;
					m_buffer_depth_rgb[3*i+1] = 255;
					m_buffer_depth_rgb[3*i+2] = lb;
					break;
				case 4:
					m_buffer_depth_rgb[3*i+0] = 0;
					m_buffer_depth_rgb[3*i+1] = 255-lb;
					m_buffer_depth_rgb[3*i+2] = 255;
					break;
				case 5:
					m_buffer_depth_rgb[3*i+0] = 0;
					m_buffer_depth_rgb[3*i+1] = 0;
					m_buffer_depth_rgb[3*i+2] = 255-lb;
					break;
				default:
					m_buffer_depth_rgb[3*i+0] = 0;
					m_buffer_depth_rgb[3*i+1] = 0;
					m_buffer_depth_rgb[3*i+2] = 0;
					break;
			}
		}
		m_new_depth_frame = true;
		m_new_depth_rgb_frame = true;
	}

	/*
	 * Puts the content of the rbg video buffer into
	 * buffer. 
	 * Returns false if there is no new frame,
	 * true if new frame is successfuly swapped in.
	 */
	bool getRGB(std::vector<uint8_t> &buffer) {
		Mutex::ScopedLock lock(m_rgb_mutex);
		if (!m_new_rgb_frame)
			return false;
		buffer.swap(m_buffer_video);
		m_new_rgb_frame = false;
		return true;
	}

	/*
	 * Puts the current frames depth values into buffer.
	 * This is the real world depth value (in cm), which is calculated
	 * each frame in DepthCallback. This is not the same as the raw non-linear
	 * disparity values streamed from the kinect. These disparity values are
	 * used to calculate these depth values.
	 * Returns false if no new frame, true if swap success.
	 */
	bool getDepth(std::vector<uint16_t> &buffer) {
		Mutex::ScopedLock lock(m_depth_mutex);
		if (!m_new_depth_frame)
			return false;
		buffer.swap(m_buffer_depth);
		m_new_depth_frame = false;
		return true;
	}

	/*
	 * Puts the disparity data, which has been modified so various depths are
	 * represented by different colors, into buffer.
	 * Returns false if no new data, true if successful swap in.
	 */
	bool getDepthRGB(std::vector<uint8_t> &buffer) {
		Mutex::ScopedLock lock(m_depth_mutex);
		if (!m_new_depth_rgb_frame)
			return false;
		buffer.swap(m_buffer_depth_rgb);
		m_new_depth_rgb_frame = false;
		return true;
	}

private:
	std::vector<uint8_t> m_buffer_depth_rgb;
	std::vector<uint16_t> m_buffer_depth;
	std::vector<uint8_t> m_buffer_video;
	std::vector<uint16_t> m_gamma;
	std::vector<uint16_t> m_gamma_disp;
	Mutex m_rgb_mutex;
	Mutex m_depth_mutex;
	bool m_new_rgb_frame;
	bool m_new_depth_frame;
	bool m_new_depth_rgb_frame;

	uint32_t depth_timestamp, video_timestamp;
};

#endif /* _MYFREENECTDECICE_H */
