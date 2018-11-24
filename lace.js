/*

Spread in height: 4 per cm
Ratio width/height can be different according to user's wishes.
Standard: 5/10 -> 2 per cm

Meaning:
    Distance between points vertically: 1/4 cm
    Distance between points horizontally: 1/2 cm

In future other patterns should be available with customized ratio support.

Currently assumes printer uses 150dpi resolution when printing

Meaning:

    Distance between points vertically: 15px
    Distance between points horizontally: 30px (for 5/10)

*/

let Lace = {};
Lace.cv = null;
Lace.scalingFactor = 1;
Lace.floodFill = null;
Lace.configuration = {
    distanceVertical: 15,
    distanceRatio: 4/10,
    borderSize: 0.08 // 5% of total width/height
};

Lace.init = function() 
{
    postMessage({msg: 'wasm'});
    let Module = {};
    Module['onRuntimeInitialized'] = function() { 
        Lace.imageChangeHandler();
    };
    Lace.cv = cv(Module);
    document.getElementById("inputCanvas").addEventListener('click', Lace.imageClickHandler);
    document.getElementById("imageInput").addEventListener('keyup', function (event) {
        if (event.keyCode === 13) 
            Lace.imageChangeHandler();
    });
}

Lace.runKMeans = function(image, k)
{
    Lace.cv.cvtColor(image, image, Lace.cv.COLOR_BGRA2BGR);

    let samples = Lace.cv.Mat.zeros(image.rows * image.cols, 3, Lace.cv.CV_8U);
    
    Lace.cv.imshow("floodfillCanvas", samples);
    
    let index = 0;
    for (let r = 0; r < image.rows; r++)
    {
        for (let c = 0; c < image.cols; c++) 
        {
            let pixel = image.ucharPtr(r, c);
            let red = pixel[0];
            let green = pixel[1];
            let blue = pixel[2];

            samples.ucharPtr(index, 0)[0] = red;
            samples.ucharPtr(index, 1)[0] = green;
            samples.ucharPtr(index, 2)[0] = blue;
            index++;
        }
    }

    Lace.cv.imshow("floodfillCanvas", samples);
            
    samples.convertTo(samples, Lace.cv.CV_32FC3, 1.0/255.0); // convert to floating point
    
    Lace.cv.imshow("floodfillCanvas", samples);

    let labels = new Lace.cv.Mat();
    let centers = new Lace.cv.Mat();
    let criteria = new Lace.cv.TermCriteria("CV_TERMCRIT_EPS"|"CV_TERMCRIT_ITER", 10, 1.0);

    Lace.cv.kmeans(samples, k, labels, criteria, 4, Lace.cv.KMEANS_PP_CENTERS, centers);

    let colors = [];
    for (let i=0; i < k; i++) {
        colors[i] = i * (255 / k);
    }

    let clustered = new Lace.cv.Mat(image.rows, image.cols, Lace.cv.CV_32F);
    for (let i=0; i < image.cols * image.rows; i++) {
        clustered.floatPtr(i / image.cols, i % image.cols)[0] = colors[labels.intAt(0,i)];
    }

    Lace.cv.imshow("floodfillCanvas", clustered);

    let returnValue = new Lace.cv.Mat();
    clustered.convertTo(returnValue, Lace.cv.CV_8U);
    Lace.cv.imshow("floodfillCanvas", returnValue);

    return returnValue;
};

Lace.addBorder = function(image) 
{
    // Create white area for border
    let borderWidth = Lace.configuration.borderSize * image.cols;
    let borderHeight = Lace.configuration.borderSize * image.rows;
    let borderType = Lace.cv.BORDER_CONSTANT;
    let dst = new Lace.cv.Mat();
    let white = new Lace.cv.Scalar(255, 255, 255, 255);
    Lace.cv.copyMakeBorder(image, dst, borderHeight, borderWidth, borderHeight, borderWidth, borderType, white);
    
    // Add border
    let borders = [{
        x1: borderWidth, 
        x2: dst.cols - borderWidth, 
        y1: borderHeight, 
        y2: dst.rows - borderHeight 
    },
    { 
        x1: borderWidth - Lace.configuration.distanceVertical, 
        x2: dst.cols - borderWidth + Lace.configuration.distanceVertical, 
        y1: borderHeight - Lace.configuration.distanceVertical, 
        y2: dst.rows - borderHeight + Lace.configuration.distanceVertical 
    }];
    let drawBorder = function(border, dst) {
        let point1 = new Lace.cv.Point(border.x1, border.y1);
        let point2 = new Lace.cv.Point(border.x2, border.y2);
        let black = new Lace.cv.Scalar(0, 0, 0, 255);
        let lineThickness = 1;
        Lace.cv.rectangle(dst, point1, point2, black, lineThickness);
    }
    for (let border of borders) {
        drawBorder(border, dst);
    }

    return dst;
}

Lace.addDottedBackground = function(source, ignoreWhereWhite) 
{
    if (ignoreWhereWhite == null)
        ignoreWhereWhite = new Lace.cv.Mat.zeros(source.size(), Lace.cv.CV_8U);

    let image = source.clone();
    let spacing_rows = 14; //Lace.configuration.distanceVertical;
    let spacing_columns = 28; //Lace.configuration.distanceVertical / Lace.configuration.distanceRatio;
    let size = 1;
    let drawCircle = false;

    for (let r = 0; r < image.size().height; r++)
    {
        for (let c = 0; c < image.size().width; c++)
        {
            drawCircle = false;

            if (r % spacing_rows == 0)
                if (c % spacing_columns == 0)
                    drawCircle = true;

            if (r % spacing_rows == spacing_rows / 2)
                if (c % spacing_columns == spacing_columns / 2)
                    drawCircle = true;

            if (drawCircle)
            {
                let point = new Lace.cv.Point(c + spacing_columns / 2, r + spacing_rows / 2);
                if (ignoreWhereWhite.ucharAt(point.y, point.x) != 255)
                    Lace.cv.circle(image, point, size, new Lace.cv.Scalar(255, 0, 0, 255), -1, Lace.cv.LINE_AA);
            }
        }
    }

    return image;

};

Lace.runFloodFill = function(image, floodFillImage, seedPoint) 
{
    // don't run floodfill on black areas (technically when "more black than white")
    let kmeans = Lace.cv.imread("kmeansCanvas");
    let sourceValue = kmeans.ucharPtr(seedPoint.y, seedPoint.x);

    if (sourceValue[0] == 128 && sourceValue[1] == 128 && sourceValue[2] == 128)
        return floodFillImage; 

    let floodImage = floodFillImage;
    let floodMask = new Lace.cv.Mat.zeros(new Lace.cv.Size(floodImage.size().width + 2, floodImage.size().height + 2), Lace.cv.CV_8U);
    let tolerance = new Lace.cv.Rect(0, 0, 0, 0);
    try {
        Lace.cv.floodFill2(floodImage, floodMask, seedPoint, new Lace.cv.Scalar(255), tolerance, new Lace.cv.Scalar(0), new Lace.cv.Scalar(0), Lace.cv.FLOODFILL_FIXED_RANGE);
    } 
    catch (ex) {
        // alert(ex);
        return floodImage;
    }
    return floodImage;
};

Lace.loadImage = function(imageElement) 
{
    // load source into canvas & recalculate scaling factor
    let sourceImage = Lace.cv.imread(imageElement);
    Lace.cv.imshow('inputCanvas', sourceImage);
    let imageWidth = document.getElementById("inputCanvas").width;
    let renderedWidth = document.getElementById("inputCanvas").getBoundingClientRect().width;
    scalingFactor = imageWidth / renderedWidth;

    // load source with dotted background
    let dotted = Lace.addDottedBackground(sourceImage);
    Lace.cv.imshow("outputCanvas", dotted);

    let result = Lace.addBorder(dotted);
    Lace.cv.imshow("resultCanvas", result);

    // load kmeans & initial mask
    floodFill = Lace.runKMeans(sourceImage, 2);
    Lace.cv.imshow("kmeansCanvas", floodFill);
    Lace.cv.imshow("floodfillCanvas", floodFill);

    // load dotted background on white canvas
    let dots = sourceImage.clone();
    dots.setTo(new Lace.cv.Scalar(255, 255, 255, 255));
    let outputDots = Lace.addDottedBackground(dots);
    Lace.cv.imshow("dotsCanvas", outputDots);
}

Lace.imageChangeHandler = function() 
{    
    var img = new Image();
    img.crossOrigin = "Anonymous";
    img.onload = function() 
    {
        Lace.loadImage(img); 
    }
    img.src = document.getElementById("imageInput").value;
}

Lace.imageClickHandler = function(event) 
{
    let sourceImage = Lace.cv.imread("inputCanvas");

    let getCoords = function(event) {
        return { 
            x: event.offsetX || event.layerX, 
            y: event.offsetY || event.layerY 
        };
    };

    let x = getCoords(event).x * scalingFactor;
    let y = getCoords(event).y * scalingFactor;

    let seedPoint = new Lace.cv.Point(x, y);
    let floodImage = Lace.runFloodFill(sourceImage, floodFill, seedPoint);
    let dotted = Lace.addDottedBackground(sourceImage, floodImage);

    let dots = sourceImage.clone();
    dots.setTo(new Lace.cv.Scalar(255, 255, 255, 255));
    dots = Lace.addDottedBackground(dots, floodImage);

    Lace.cv.imshow("dotsCanvas", dots);
    Lace.cv.imshow("floodfillCanvas", floodFill);
    Lace.cv.imshow("outputCanvas", dotted);

    let result = Lace.addBorder(dotted);
    Lace.cv.imshow("resultCanvas", result);
}

Lace.init();