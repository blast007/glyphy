/*
 * Copyright © 2011  Google, Inc.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 * Google Author(s): Behdad Esfahbod, Maysum Panju
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <cairo-ft.h>

#include "geometry.hh"
#include "cairo-helper.hh"
#include "freetype-helper.hh"
#include "sample-curves.hh"
#include "bezier-arc-approximation.hh"


using namespace std;
using namespace Geometry;
using namespace CairoHelper;
using namespace FreeTypeHelper;
using namespace SampleCurves;
using namespace BezierArcApproximation;

typedef Vector<Coord> vector_t;
typedef Point<Coord> point_t;
typedef Line<Coord> line_t;
typedef Circle<Coord, Scalar> circle_t;
typedef Arc<Coord, Scalar> arc_t;
typedef Bezier<Coord> bezier_t;


static void 
closest_arcs_to_square (Point<Coord> square_top_left, 
                        Scalar side_length, 
                        vector <Arc <Coord, Scalar> > arc_list,
                        vector <Arc <Coord, Scalar> > &closest_arcs)
{
  Point<Coord> center (square_top_left.x + side_length / 2., 
                       square_top_left.y + side_length / 2.);
  double min_distance = INFINITY;
  int k;
  for (k = 0; k < arc_list.size (); k++) 
    min_distance = std::min (min_distance, arc_list.at(k).distance_to_point (center));
    
  /* If d is the distance from the center of the square to the nearest arc, then
     all nearest arcs to the square must be at most [d + s/sqrt(2)] from the center. */
  Scalar radius = min_distance + (side_length / sqrt (2));
  for (k = 0; k < arc_list.size (); k++) {
    if (arc_list.at(k).distance_to_point (center) < radius)
      closest_arcs.push_back (arc_list.at (k));
  }
}


static double
distance_to_an_arc (Point<Coord> square_top_left,
                    vector <Arc <Coord, Scalar> > arc_list)
{
  double min_distance = INFINITY;
  int k;
  for (k = 0; k < arc_list.size (); k++)
    min_distance = std::min (min_distance, arc_list.at(k).distance_to_point (square_top_left));
    
  return min_distance;
}
                     


static void 
gridify_and_find_arcs (cairo_t *cr, vector <Arc <Coord, Scalar> > arc_list)
{


  double box_width = 5;
  double grid_min_x = INFINITY; //-20;
  double grid_max_x = -1 * INFINITY; //350;
  double grid_min_y = INFINITY; //-120;
  double grid_max_y = -1 * INFINITY; //210;
  
 /* 
  for (int i = 0; i < arc_list.size (); i++)  {
    grid_min_x = std::min (grid_min_x, arc_list.at(i).center().x - arc_list.at(i).radius());
    grid_max_x = std::max (grid_max_x, arc_list.at(i).center().x + arc_list.at(i).radius());
    grid_min_y = std::min (grid_min_y, arc_list.at(i).center().y - arc_list.at(i).radius());
    grid_max_y = std::max (grid_max_y, arc_list.at(i).center().y + arc_list.at(i).radius());
  }

  box_width = std::min (grid_max_x - grid_min_x, grid_max_y - grid_min_y) / 1000;
  grid_min_x -= 20 * box_width;
  grid_min_y -= 20 * box_width;
  grid_max_x += 20 * box_width;
  grid_max_y += 20 * box_width;
  
  printf("Grid: [%g, %g] x [%g, %g] with box width %g.\n", grid_min_x, grid_max_x, grid_min_y, grid_max_y, box_width); */
 
 
 grid_min_x = 15;
    grid_max_x = 25;
    grid_min_y = -3;
    grid_max_y = 5;
    box_width = 0.01;
  for (double i = grid_min_x; i < grid_max_x; i+= box_width)
  {
    for (double j = grid_min_y; j < grid_max_y; j+= box_width) {
      vector <Arc <Coord, Scalar> > closest_arcs;
      closest_arcs_to_square (Point<Coord> (i, j), 
                              box_width, arc_list, closest_arcs);
      double gradient = closest_arcs.size () * 5. / arc_list.size ();
      cairo_set_source_rgb (cr, 0.8 * gradient, 1.7 * gradient, 1.1 * gradient);
      CairoHelper::cairo_demo_point (cr, Point<Coord> (i + box_width * 0.5 , j + box_width * 0.5));
    }
  }
  
}


static void
draw_distance_field (cairo_t *cr, vector <Arc <Coord, Scalar> > arc_list)
{

  double box_width = 5;
  double grid_min_x = INFINITY; //-20;
  double grid_max_x = -1 * INFINITY; //350;
  double grid_min_y = INFINITY; //-120;
  double grid_max_y = -1 * INFINITY; //210;
  
   grid_min_x = 1000; //15;
    grid_max_x = 40000; //25;
    grid_min_y = -20000; //-3;
    grid_max_y = 30000; //5;
    box_width = 100; //0.01;
  for (double i = grid_min_x; i < grid_max_x; i+= box_width)
  {
    for (double j = grid_min_y; j < grid_max_y; j+= box_width) {
      double gradient = distance_to_an_arc (Point<Coord> (i, j), arc_list) / 2000;
      cairo_set_source_rgb (cr, gradient * 1., gradient * 1.1, gradient * 1.2);
      CairoHelper::cairo_demo_point (cr, Point<Coord> (i + box_width * 0.5 , j + box_width * 0.5));
    }
  }
}

static void
demo_curve (cairo_t *cr, const bezier_t &b)
{
  cairo_save (cr);
  cairo_set_line_width (cr, 5);

  cairo_curve (cr, b);
  cairo_set_viewport (cr);
  cairo_new_path (cr);

  cairo_set_source_rgb (cr, 1, 0, 0);
  cairo_demo_curve (cr, b);

  typedef MaxDeviationApproximatorExact MaxDev;
  typedef BezierArcErrorApproximatorBehdad<MaxDev> BezierArcError;
  typedef BezierArcApproximatorMidpointTwoPart<BezierArcError> BezierArcApproximator;
  typedef BezierArcsApproximatorSpringSystem<BezierArcApproximator> SpringSystem;

  double tolerance = .00001;
  double e;
  std::vector<Arc<Coord, Scalar> > &arcs = SpringSystem::approximate_bezier_with_arcs (b, tolerance, &e);

  printf ("Num arcs %d; Approximation error %g; Tolerance %g; Percentage %g; %s\n",
	  (int) arcs.size (), e, tolerance, round (100 * e / tolerance), e <= tolerance ? "PASS" : "FAIL");

   gridify_and_find_arcs (cr, arcs);
   cairo_set_source_rgb (cr, 0.9, 0.9, 0.9);
  cairo_demo_curve (cr, b);

  cairo_set_source_rgba (cr, 1.0, 0.2, 0.2, 1.0);
  cairo_set_line_width (cr, cairo_get_line_width (cr) / 2);
  for (unsigned int i = 0; i < arcs.size (); i++)
    cairo_demo_arc (cr, arcs[i]);

 

  delete &arcs;

  cairo_restore (cr);
}

static void
demo_text (cairo_t *cr, const char *family, const char *utf8)
{
  cairo_save (cr);
  cairo_select_font_face (cr, family, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_line_width (cr, 5);
#define FONT_SIZE 2048
  cairo_set_font_size (cr, FONT_SIZE);

  cairo_text_path (cr, utf8);
  cairo_path_t *path = cairo_copy_path (cr);
  cairo_path_print_stats (path);
  cairo_path_destroy (path);
  cairo_set_viewport (cr);

  cairo_new_path (cr);
  cairo_set_source_rgb (cr, 1, 0, 0);
  cairo_text_path (cr, utf8);
  cairo_fill (cr);

  typedef MaxDeviationApproximatorExact MaxDev;
  typedef BezierArcErrorApproximatorBehdad<MaxDev> BezierArcError;
  typedef BezierArcApproximatorMidpointTwoPart<BezierArcError> BezierArcApproximator;
  typedef BezierArcsApproximatorSpringSystem<BezierArcApproximator> SpringSystem;
  typedef ArcApproximatorOutlineSink<SpringSystem> ArcApproximatorOutlineSink;

  double e;

  class ArcAccumulator
  {
    public:

    static bool callback (const arc_t &arc, void *closure)
    {
       ArcAccumulator *acc = static_cast<ArcAccumulator *> (closure);
       acc->arcs.push_back (arc);
       return true;
    }

    std::vector<arc_t> arcs;
  } acc;

  FT_Face face = cairo_ft_scaled_font_lock_face (cairo_get_scaled_font (cr));
  unsigned int upem = face->units_per_EM;
  FT_Set_Char_Size (face, upem*64, upem*64, 0, 0);
  double tolerance = upem * 64. / 256;
  if (FT_Load_Glyph (face, FT_Get_Char_Index (face, (FT_ULong) *utf8), FT_LOAD_NO_BITMAP))
    abort ();
  assert (face->glyph->format == FT_GLYPH_FORMAT_OUTLINE);
  //ArcApproximatorOutlineSink::approximate_glyph (&face->glyph->outline, acc.callback, static_cast<void *> (&acc), tolerance, &e);
  ArcApproximatorOutlineSink outline_arc_approximator (acc.callback,
						       static_cast<void *> (&acc),
						       tolerance);
  FreeTypeOutlineSource<ArcApproximatorOutlineSink>::decompose_outline (&face->glyph->outline,
									outline_arc_approximator);
  cairo_ft_scaled_font_unlock_face (cairo_get_scaled_font (cr));


  printf ("Num arcs %d; Approximation error %g; Tolerance %g; Percentage %g; %s\n",
	  (int) acc.arcs.size (), e, tolerance, round (100 * e / tolerance), e <= tolerance ? "PASS" : "FAIL");

  double x = 0, y = 0;
  double dx = 1, dy = 1;
  cairo_user_to_device (cr, &x, &y);
  cairo_user_to_device_distance (cr, &dx, &dy);
  cairo_identity_matrix (cr);
  cairo_translate (cr, x, y);
  cairo_scale (cr, FONT_SIZE*dx/(upem*64), -FONT_SIZE*dy/(upem*64));

  cairo_set_source_rgba (cr, 0.0, 0.0, 1.0, .5);
  point_t start (0, 0);
  for (unsigned int i = 0; i < acc.arcs.size (); i++) {
    if (!cairo_has_current_point (cr))
      start = acc.arcs[i].p0;
    cairo_arc (cr, acc.arcs[i]);
    if (acc.arcs[i].p1 == start) {
      cairo_close_path (cr);
      cairo_new_sub_path (cr);
    }
  }
  cairo_fill (cr);

  cairo_set_source_rgba (cr, 0.0, 1.0, 0.0, .3);
  cairo_set_line_width (cr, 4*64);
  for (unsigned int i = 0; i < acc.arcs.size (); i++)
    cairo_demo_arc (cr, acc.arcs[i]);

  gridify_and_find_arcs (cr, acc.arcs);
  
  cairo_restore (cr);
}








int main (int argc, char **argv)
{
  cairo_t *cr;
  char *filename;
  cairo_status_t status;
  cairo_surface_t *surface;

  if (argc != 2) {
    fprintf (stderr, "Usage: arc OUTPUT_FILENAME\n");
    return 1;
  }

  filename = argv[1];

  surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
					1400, 800);
  cr = cairo_create (surface);

  cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
  cairo_paint (cr);

 // demo_curve (cr, sample_curve_skewed ());
 // demo_curve (cr, sample_curve_riskus_simple ());   
  demo_curve (cr, sample_curve_riskus_complicated ());
 // demo_curve (cr, sample_curve_s ());
 // demo_curve (cr, sample_curve_serpentine_c_symmetric ());
 // demo_curve (cr, sample_curve_serpentine_s_symmetric ());  //x
 // demo_curve (cr, sample_curve_serpentine_quadratic ());
 // demo_curve (cr, sample_curve_loop_cusp_symmetric ());  
 // demo_curve (cr, sample_curve_loop_gamma_symmetric ());
 // demo_curve (cr, sample_curve_loop_gamma_small_symmetric ());
 // demo_curve (cr, sample_curve_loop_o_symmetric ());  // x
 // demo_curve (cr, sample_curve_semicircle_top ());
 // demo_curve (cr, sample_curve_semicircle_bottom ());
 // demo_curve (cr, sample_curve_semicircle_left ());
 // demo_curve (cr, sample_curve_semicircle_right ());
 
 // demo_text (cr, "Times", "g");

  cairo_destroy (cr);

  status = cairo_surface_write_to_png (surface, filename);
  cairo_surface_destroy (surface);

  if (status != CAIRO_STATUS_SUCCESS) {
    fprintf (stderr, "Could not save png to '%s'\n", filename);
    return 1;
  }

  return 0;
}