let _cv;

let runKMeans = function(image, k)
{
    _cv.cvtColor(image, image, _cv.COLOR_BGRA2BGR);

    let samples = _cv.Mat.zeros(image.rows * image.cols, 3, _cv.CV_8U);
    
    _cv.imshow("floodfillCanvas", samples);
    
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

    _cv.imshow("floodfillCanvas", samples);
            
    samples.convertTo(samples, _cv.CV_32FC3, 1.0/255.0); // convert to floating point
    
    _cv.imshow("floodfillCanvas", samples);

    let labels = new _cv.Mat();
    let centers = new _cv.Mat();
    let criteria = new _cv.TermCriteria("CV_TERMCRIT_EPS"|"CV_TERMCRIT_ITER", 10, 1.0);

    _cv.kmeans(samples, k, labels, criteria, 4, _cv.KMEANS_PP_CENTERS, centers);

    let colors = [];
    for (let i=0; i < k; i++) {
        colors[i] = i * (255 / k);
    }

    let clustered = new _cv.Mat(image.rows, image.cols, _cv.CV_32F);
    for (let i=0; i < image.cols * image.rows; i++) {
        clustered.floatPtr(i / image.cols, i % image.cols)[0] = colors[labels.intAt(0,i)];
    }

    _cv.imshow("floodfillCanvas", clustered);

    let returnValue = new _cv.Mat();
    clustered.convertTo(returnValue, _cv.CV_8U);
    _cv.imshow("floodfillCanvas", returnValue);

    return returnValue;
};

let addDottedBackground = function(source, ignoreWhereWhite) 
{
    if (ignoreWhereWhite == null)
        ignoreWhereWhite = new _cv.Mat.zeros(source.size(), _cv.CV_8U);

    let image = source.clone();
    let spacing_rows = 8;
    let spacing_columns = 16;
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
                let point = new _cv.Point(c + spacing_columns / 2, r + spacing_rows / 2);
                if (ignoreWhereWhite.ucharAt(point.y, point.x) != 255)
                    _cv.circle(image, point, size, new _cv.Scalar(255, 0, 0, 255), -1, _cv.LINE_AA);
            }
        }
    }

    return image;

};

let runFloodFill = function(image, floodFillImage, seedPoint) 
{
    // don't run floodfill on black areas (technically when "more black than white")
    let kmeans = _cv.imread("kmeansCanvas");
    let sourceValue = kmeans.ucharPtr(seedPoint.y, seedPoint.x);

    if (sourceValue[0] == 128 && sourceValue[1] == 128 && sourceValue[2] == 128)
        return floodFillImage; 

    let floodImage = floodFillImage;
    let floodMask = new _cv.Mat.zeros(new _cv.Size(floodImage.size().width + 2, floodImage.size().height + 2), _cv.CV_8U);
    let tolerance = new _cv.Rect(0, 0, 0, 0);
    try {
        _cv.floodFill2(floodImage, floodMask, seedPoint, new _cv.Scalar(255), tolerance, new _cv.Scalar(0), new _cv.Scalar(0), _cv.FLOODFILL_FIXED_RANGE);
    } 
    catch (ex) {
        // alert(ex);
        return floodImage;
    }
    return floodImage;
};

let loadImage = function(imageElement) {

    // load source into canvas & recalculate scaling factor
    let sourceImage = _cv.imread(imageElement);
    _cv.imshow('inputCanvas', sourceImage);
    let imageWidth = document.getElementById("inputCanvas").width;
    let renderedWidth = document.getElementById("inputCanvas").getBoundingClientRect().width;
    scalingFactor = imageWidth / renderedWidth;

    // load source with dotted background
    let dotted = addDottedBackground(sourceImage);
    _cv.imshow("outputCanvas", dotted);

    // load kmeans & initial mask
    floodFill = runKMeans(sourceImage, 2);
    _cv.imshow("kmeansCanvas", floodFill);
    _cv.imshow("floodfillCanvas", floodFill);

    // load dotted background on white canvas
    let dots = sourceImage.clone();
    dots.setTo(new _cv.Scalar(255,255,255));
    let outputDots = addDottedBackground(dots);
    _cv.imshow("dotsCanvas", outputDots);
}

let imageChangeHandler = function() 
{    
    var img = new Image();
    img.crossOrigin = "Anonymous";
    img.onload = function() 
    {
        loadImage(img); 
    }
    img.src = document.getElementById("imageInput").value;
}

let imageClickHandler = function(event) { 

    let sourceImage = _cv.imread("inputCanvas");

    let getCoords = function(event) {
        return { 
            x: event.offsetX || event.layerX, 
            y: event.offsetY || event.layerY 
        };
    };

    let x = getCoords(event).x * scalingFactor;
    let y = getCoords(event).y * scalingFactor;

    let seedPoint = new _cv.Point(x, y);
    let floodImage = runFloodFill(sourceImage, floodFill, seedPoint);
    let result = addDottedBackground(sourceImage, floodImage);

    let dots = sourceImage.clone();
    dots.setTo(new _cv.Scalar(255,255,255));
    dots = addDottedBackground(dots, floodImage);

    _cv.imshow("dotsCanvas", dots);
    _cv.imshow("floodfillCanvas", floodFill);
    _cv.imshow("outputCanvas", result);
}

let init = function() {

    postMessage({msg: 'wasm'});
    let Module = {};
    let scalingFactor = 1;
    let floodFill = null;
    Module['onRuntimeInitialized'] = function() { 
        imageChangeHandler();
    };
    _cv = cv(Module);
    document.getElementById("imageInput").addEventListener('keyup', function (event) {
        if (event.keyCode === 13) 
            imageChangeHandler();
    });
    document.getElementById("inputCanvas").addEventListener('click', imageClickHandler);
}

init();