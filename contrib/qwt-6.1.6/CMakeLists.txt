project(qwt)

find_package(Qt5 COMPONENTS Widgets OpenGL PrintSupport Concurrent Svg)
find_package(OpenGL)

add_library(qwt 
    src/qwt_scale_engine.cpp
    src/qwt_abstract_legend.cpp
    src/qwt_abstract_scale_draw.cpp
    src/qwt_abstract_scale.cpp
    src/qwt_abstract_slider.cpp
    src/qwt_analog_clock.cpp
    src/qwt_arrow_button.cpp
    src/qwt_clipper.cpp
    src/qwt_color_map.cpp
    src/qwt_column_symbol.cpp
    src/qwt_compass_rose.cpp
    src/qwt_compass.cpp
    src/qwt_counter.cpp
    src/qwt_curve_fitter.cpp
    src/qwt_date_scale_draw.cpp
    src/qwt_date_scale_engine.cpp
    src/qwt_date.cpp
    src/qwt_dial_needle.cpp
    src/qwt_dial.cpp
    src/qwt_dyngrid_layout.cpp
    src/qwt_event_pattern.cpp
    src/qwt_graphic.cpp
    src/qwt_interval_symbol.cpp
    src/qwt_interval.cpp
    src/qwt_knob.cpp
    src/qwt_legend_data.cpp
    src/qwt_legend_label.cpp
    src/qwt_legend.cpp
    src/qwt_magnifier.cpp
    src/qwt_math.cpp
    src/qwt_matrix_raster_data.cpp
    src/qwt_null_paintdevice.cpp
    src/qwt_painter_command.cpp
    src/qwt_painter.cpp
    src/qwt_panner.cpp
    src/qwt_picker_machine.cpp
    src/qwt_picker.cpp
    src/qwt_pixel_matrix.cpp
    src/qwt_plot_abstract_barchart.cpp
    src/qwt_plot_axis.cpp
    src/qwt_plot_canvas.cpp
    src/qwt_plot_curve.cpp
    src/qwt_plot_dict.cpp
    src/qwt_plot_directpainter.cpp
    src/qwt_plot_glcanvas.cpp
    src/qwt_plot_grid.cpp
    src/qwt_plot_histogram.cpp
    src/qwt_plot_intervalcurve.cpp
    src/qwt_plot_item.cpp
    src/qwt_plot_layout.cpp
    src/qwt_plot_legenditem.cpp
    src/qwt_plot_magnifier.cpp
    src/qwt_plot_marker.cpp
    src/qwt_plot_multi_barchart.cpp
    src/qwt_plot_panner.cpp
    src/qwt_plot_picker.cpp
    src/qwt_plot_rasteritem.cpp
    src/qwt_plot_renderer.cpp
    src/qwt_plot_rescaler.cpp
    src/qwt_plot_scaleitem.cpp
    src/qwt_plot_seriesitem.cpp
    src/qwt_plot_shapeitem.cpp
    src/qwt_plot_spectrocurve.cpp
    src/qwt_plot_svgitem.cpp
    src/qwt_plot_textlabel.cpp
    src/qwt_plot_tradingcurve.cpp
    src/qwt_plot_xml.cpp
    src/qwt_plot_zoneitem.cpp
    src/qwt_plot_zoomer.cpp
    src/qwt_plot.cpp
    src/qwt_point_3d.cpp
    src/qwt_point_data.cpp
    src/qwt_point_mapper.cpp
    src/qwt_point_polar.cpp
    src/qwt_raster_data.cpp
    src/qwt_round_scale_draw.cpp
    src/qwt_sampling_thread.cpp
    src/qwt_scale_div.cpp
    src/qwt_scale_draw.cpp
    src/qwt_scale_engine.cpp
    src/qwt_scale_map.cpp
    src/qwt_scale_widget.cpp
    src/qwt_series_data.cpp
    src/qwt_slider.cpp
    src/qwt_spline.cpp
    src/qwt_symbol.cpp
    src/qwt_system_clock.cpp
    src/qwt_text_engine.cpp
    src/qwt_text_label.cpp
    src/qwt_text.cpp
    src/qwt_thermo.cpp
    src/qwt_transform.cpp
    src/qwt_wheel.cpp
    src/qwt_widget_overlay.cpp
)

#target_include_directories(qwt PRIVATE 
#    ${Qt5Widgets_INCLUDE_DIRS} 
#    ${Qt5OpenGL_INCLUDE_DIRS} 
#    ${Qt5PrintSupport_INCLUDE_DIRS} 
#    ${Qt5Concurrent_INCLUDE_DIRS}
#    ${Qt5Svg_INCLUDE_DIRS})

#target_include_directories(qwt PRIVATE ${QT5_INCLUDE_DIRS})

target_link_libraries(qwt Qt5::Widgets Qt5::PrintSupport Qt5::OpenGL Qt5::Concurrent Qt5::Svg ${OPENGL_LIBRARIES})

set_property(TARGET qwt PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET qwt PROPERTY AUTOMOC ON)
