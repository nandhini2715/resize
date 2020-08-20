const isNodeJs = (typeof window) === 'undefined'? true : false;

if　(isNodeJs)　{
  var Benchmark = require('benchmark');
  var cv = require('../../opencv');
  var HelpFunc = require('../perf_helpfunc');
  var Base = require('../base');
} else {
  var paramsElement = document.getElementById('params');
  var runButton = document.getElementById('runButton');
  var logElement = document.getElementById('log');
}

function perf() {

    console.log('opencv.js loaded');
    if (isNodeJs) {
      global.cv = cv;
      global.combine = HelpFunc.combine;
      global.cvtStr2cvSize = HelpFunc.cvtStr2cvSize;
      global.cvSize = Base.getCvSize();
    } else {
      enableButton();
      cvSize = getCvSize();
    }
    let totalCaseNum, currentCaseId;

    const WarpPersSize = [cvSize.szVGA, cvSize.sz720p, cvSize.sz1080p];
    const InterType = ["INTER_NEAREST", "INTER_LINEAR"];
    const BorderMode = ["BORDER_CONSTANT", "BORDER_REPLICATE"]
    const combiWarpPers = combine(WarpPersSize, InterType, BorderMode);

    function addWarpPerspectiveCase(suite, type) {
        suite.add('warpPerspective', function() {
            cv.warpPerspective(src, dst, warpMat, sz, interType, borderMode, borderColor);
          }, {
              'setup': function() {
                let sz = this.params.size;
                let interType = cv[this.params.interType];
                let borderMode = cv[this.params.borderMode];
                let srcSize = new cv.Size(512, 512);

                let borderColor = new cv.Scalar.all(150);
                let src = new cv.Mat(srcSize, cv.CV_8UC4);
                let dst = new cv.Mat(sz, cv.CV_8UC4);
                //fillGradient
                let delta = 5;
                let ch = 4;
                let n = 255 / delta;
                let r, c, i, view;
                for(r=0; r<src.rows; r++)
                {
                    let kR = r % (2*n);
                    let valR = (kR<=n) ? delta*kR : delta*(2*n-kR);
                    for(c=0; c<src.cols; c++)
                    {
                        let kC = c % (2*n);
                        let valC = (kC<=n) ? delta*kC : delta*(2*n-kC);
                        let vals = [valR, valC, (200*r/src.rows), 255];
                        view = src.ucharPtr(r, c);
                        for(i=0; i<ch; i++) view[i] = vals[i];
                    }
                }

                if (borderMode == cv.BORDER_CONSTANT) {
                  //smoothBorder
                  delta = 1;
                  ch = 4;
                  n = 100/delta;
                  let nR = Math.min(n, (src.rows+1)/2);
                  let nC = Math.min(n, (src.cols+1)/2);
                  let index;

                  for (r=0; r<nR; r++) {
                    let k1 = r*delta/100.0, k2 = 1-k1;
                    for(c=0; c<src.cols; c++) {
                      view = src.ucharPtr(r, c);
                      for(index = 0; index <4; index++) {
                        temp = view[index]*k1 + borderColor[index] * k2;
                        view[index] = temp;
                      }
                    }
                    for(c=0; c<src.cols; c++) {
                      view = src.ucharPtr(src.rows-r-1, c);
                      for(index = 0; index <4; index++) {
                        temp = view[index]*k1 + borderColor[index] * k2;
                        view[index] = temp;
                      }
                    }
                  }
                  for (r=0; r<src.rows; r++) {
                    for(c=0; c<nC; c++) {
                      let k1 = c*delta/100.0, k2 = 1-k1;
                      view = src.ucharPtr(r, c);
                      for(index = 0; index <4; index++) {
                        temp = view[index]*k1 + borderColor[index] * k2;
                        view[index] = temp;
                      }
                    }
                    for(c=0; c<n; c++) {
                      let k1 = c*delta/100.0, k2 = 1-k1;
                      view = src.ucharPtr(r, src.cols-c-1);
                      for(index = 0; index <4; index++) {
                        temp = view[index]*k1 + borderColor[index] * k2;
                        view[index] = temp;
                      }
                    }
                  }

                }

                let rotMat = cv.getRotationMatrix2D(new cv.Point(src.cols/2.0, src.rows/2.0), 30.0, 2.2);
                let warpMat = new cv.Mat(3, 3, cv.CV_64FC1);

                for(r=0; r<2; r++) {
                  for(c=0; c<3; c++) {
                    view = warpMat.doublePtr(r,c)
                    view[0] = rotMat.doubleAt(r, c);
                  }
                }
                view = warpMat.doublePtr(2,0);
                view[0] = 0.3/sz.width;
                view = warpMat.doublePtr(2,1);
                view[0] = 0.3/sz.height;
                view = warpMat.doublePtr(2,2);
                view[0] = 1;
                },
              'teardown': function() {
                src.delete();
                dst.delete();
                warpMat.delete();
              }
          });
    }

    function addWarpPerspectiveModeCase(suite, combination, type) {
      totalCaseNum += combination.length;
      for (let i = 0; i < combination.length; ++i) {
        let size =  combination[i][0];
        let interType = combination[i][1];
        let borderMode = combination[i][2];

        let params = {size: size, interType:interType, borderMode:borderMode};
        addKernelCase(suite, params, type, addWarpPerspectiveCase);
      }
    }

    function genBenchmarkCase(paramsContent) {
      let suite = new Benchmark.Suite;
      totalCaseNum = 0;
      currentCaseId = 0;

      if (/\([0-9]+x[0-9]+,[\ ]*INTER\_\w+,[\ ]*BORDER\_\w+\)/g.test(paramsContent.toString())) {
          let params = paramsContent.toString().match(/\([0-9]+x[0-9]+,[\ ]*INTER\_\w+,[\ ]*BORDER\_\w+\)/g)[0];
          let paramObjs = [];
          paramObjs.push({name:"size", value:"", reg:[""], index:0});
          paramObjs.push({name:"interType", value: "", reg:["/INTER\_\\w+/"], index:1});
          paramObjs.push({name:"borderMode", value: "", reg:["/BORDER\_\\w+/"], index:2});
          let locationList = decodeParams2Case(params, paramObjs, warpPersCombinations);

          for (let i = 0; i < locationList.length; i++){
              let first = locationList[i][0];
              let second = locationList[i][1];
              addWarpPerspectiveModeCase(suite, [warpPersCombinations[first][second]], first);
            }
      } else {
        log("no filter or getting invalid params, run all the cases");
        addWarpPerspectiveModeCase(suite, combiWarpPers, 0);
      }
      setBenchmarkSuite(suite, "warpPerspective", currentCaseId);
      log(`Running ${totalCaseNum} tests from warpPerspective`);
      suite.run({ 'async': true }); // run the benchmark
  }

    let warpPersCombinations = [combiWarpPers];

    if (isNodeJs) {
      const args = process.argv.slice(2);
      let paramsContent = '';
      if (/--test_param_filter=\([0-9]+x[0-9]+,[\ ]*INTER\_\w+,[\ ]*BORDER\_\w+\)/g.test(args.toString())) {
        paramsContent = args.toString().match(/\([0-9]+x[0-9]+,[\ ]*INTER\_\w+,[\ ]*BORDER\_\w+\)/g)[0];
      }
      genBenchmarkCase(paramsContent);
    } else {
      runButton.onclick = function()　{
        let paramsContent = paramsElement.value;
        genBenchmarkCase(paramsContent);
        if (totalCaseNum !== 0) {
          disableButton();
        }
      }
    }
};

async function main() {
  if (cv instanceof Promise) {
    cv = await cv;
    perf();
  } else {
    cv.onRuntimeInitialized = perf;
  }
}

main();