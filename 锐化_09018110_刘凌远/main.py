from PIL import Image
import numpy as np


def input_img(name: str):
    '''
    输入图像
    '''
    return np.array(Image.open(name), dtype=np.uint8)


def output_img(img, name: str):
    '''
    保存图像
    '''
    Image.fromarray(img).save(name)


def unsharp_masking(img):
    core = np.array(
        ((0, -1, 0),
         (-1, 5, -1),
         (0, -1, 0))
    )
    # 超出的部分取0
    # 因为在左上部分超出的是-1，会取到右下的边，所以只用补右下的0
    enlarged_img = np.zeros(
        (img.shape[0] + 1, img.shape[1] + 1), dtype=np.uint8)
    for i in range(img.shape[0]):
        for j in range(img.shape[1]):
            enlarged_img[i][j] = img[i][j]

    result = np.zeros(img.shape, dtype=np.uint8)
    for i in range(img.shape[0]):
        for j in range(img.shape[1]):
            img_slice = np.array(
                ((enlarged_img[i-1][j-1], enlarged_img[i-1]
                  [j], enlarged_img[i-1][j+1]),
                 (enlarged_img[i][j-1], enlarged_img[i]
                  [j], enlarged_img[i][j+1]),
                 (enlarged_img[i+1][j-1], enlarged_img[i+1]
                  [j], enlarged_img[i+1][j+1])), dtype=np.uint8
            )  # 以[i][j]为中心，取3*3的窗口
            result[i][j] = np.sum(img_slice * core)
    return result


if __name__ == "__main__":
    img = input_img("./lung.bmp")
    # print(img)
    result = unsharp_masking(img)
    output_img(result, "out.bmp")
