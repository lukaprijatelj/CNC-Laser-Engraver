using System;
using System.Collections.Generic;
using System.Drawing;


namespace PointSequence
{
    public class PointSequence : List<Point>
    {
        Bitmap image;
        List<Point> redPixels = new List<Point>(1000);

        public PointSequence(String imagePath)
        {
            using (image = new Bitmap(imagePath))
            {
                FindEdge();
                FindPath();
            }
        }

        // RED - Edge detection
        private void FindEdge()
        {
            for (int x = 0; x < image.Width; x++)
            {
                for (int y = 0; y < image.Height; y++)
                {
                    Color color = image.GetPixel(x, y);
                    if (color.R == Color.Black.R && color.G == Color.Black.G && color.B == Color.Black.B && color.A == Color.Black.A)
                    {
                        for (int i = 0; i < 4; i++)
                        {
                            int nx = -1;
                            int ny = -1;
                            switch (i)
                            {
                                case 0: nx = x; ny = y - 1; break;
                                case 1: nx = x; ny = y + 1; break;
                                case 2: nx = x - 1; ny = y; break;
                                case 3: nx = x + 1; ny = y; break;
                            }
                            if (0 <= nx && nx < image.Width && 0 <= ny && ny < image.Height)
                            {
                                Color colorNext = image.GetPixel(nx, ny);
                                if (colorNext.R == Color.White.R && colorNext.G == Color.White.G && colorNext.B == Color.White.B && colorNext.A == Color.White.A)
                                {
                                    redPixels.Add(new Point(x, y));
                                    image.SetPixel(x, y, Color.Red);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        // GREEN - Path
        private void FindPath()
        {
            int cx = 0;
            int cy = 0;
            while (redPixels.Count > 0)
            {
                Point? pointMin = null;
                double deltaMin = double.MaxValue;
                foreach (Point point in redPixels)
                {
                    int deltaX = Math.Abs(cx - point.X);
                    int deltaY = Math.Abs(cy - point.Y);
                    double delta = Math.Sqrt(Math.Pow(deltaX, 2) + Math.Pow(deltaY, 2));
                    if (deltaMin > delta)
                    {
                        deltaMin = delta;
                        pointMin = point;
                    }
                }
                redPixels.Remove(pointMin.Value);
                cx = pointMin.Value.X;
                cy = pointMin.Value.Y;
                image.SetPixel(cx, cy, Color.Green);
                Point cutPoint = new Point(cx, cy);
                this.Add(cutPoint);
            }
        }
    }
}
