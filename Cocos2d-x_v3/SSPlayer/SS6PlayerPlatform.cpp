﻿// 
//  SS6Platform.cpp
//
#include "SS6PlayerPlatform.h"

/**
* 各プラットフォームに合わせて処理を作成してください
* OpenGL+glut用に作成されています。
*/

namespace ss
{

	//テクスチャ管理クラス
	#define TEXTURE_MAX (512)				//全プレイヤーで使えるのテクスチャ枚数
	cocos2d::Texture2D* texture[TEXTURE_MAX];		//テクスチャ情報の保持
	int texture_index = 0;					//手k数茶情報の参照ポインタ

	//座標系設定
	int _direction;
	int _window_w;
	int _window_h;
    bool _isContentScaleFactorAuto;

	#define OPENGLES20	(1)	//Opengl 2.0で動作するコードにする場合は1

	//アプリケーション初期化時の処理
	void SSPlatformInit(void)
	{
		memset(texture, 0, sizeof(texture));
		texture_index = 0;

		_direction = PLUS_UP;
		_window_w = 1280;
		_window_h = 720;
        _isContentScaleFactorAuto = true;
	}
	//アプリケーション終了時の処理
	void SSPlatformRelese(void)
	{
		int i;
		for (i = 0; i < TEXTURE_MAX; i++)
		{
			SSTextureRelese(i);
		}
	}

	/**
	* 上下どちらを正方向にするかとウィンドウサイズを設定します.
	* 上が正の場合はPLUS_UP、下が正の場合はPLUS_DOWN
	*
	* @param  direction      プラス方向
	* @param  window_w       ウィンドウサイズ
	* @param  window_h       ウィンドウサイズ
	*/
	void SSSetPlusDirection(int direction, int window_w, int window_h)
	{
		_direction = direction;
		_window_w = window_w;
		_window_h = window_h;
	}
	void SSGetPlusDirection(int &direction, int &window_w, int &window_h)
	{
		direction = _direction;
		window_w = _window_w;
		window_h = _window_h;
	}

    //cocos2d-xのコンテンツスケールに合わせてUVを補正する
    void SSSetPlusDirection(bool flag)
    {
        _isContentScaleFactorAuto = flag;
    }

	/**
	* ファイル読み込み
	*/
	unsigned char* SSFileOpen(const char* pszFileName, const char* pszMode, unsigned long * pSize)
	{
        std::string fullpath = cocos2d::FileUtils::getInstance()->fullPathForFilename(pszFileName);
        
        ssize_t nSize = 0;
        void* loadData = cocos2d::FileUtils::getInstance()->getFileData(fullpath, pszMode, &nSize);
        if (loadData == nullptr)
        {
            std::string msg = "Can't load project data > " + fullpath;
            CCASSERT(loadData != nullptr, msg.c_str());
        }
        *pSize = (long)nSize;

		return (unsigned char *)loadData;
	}

	/**
	* テクスチャの読み込み
	*/
	long SSTextureLoad(const char* pszFileName, int  wrapmode, int filtermode)
	{
		/**
		* テクスチャ管理用のユニークな値を返してください。
		* テクスチャの管理はゲーム側で行う形になります。
		* テクスチャにアクセスするハンドルや、テクスチャを割り当てたバッファ番号等になります。
		*
		* プレイヤーはここで返した値とパーツのステータスを引数に描画を行います。
		*/
		long rc = 0;

		//空きバッファを検索して使用する
		int start_index = texture_index;	//開始したインデックスを保存する
		bool exit = true;
		bool isLoad = false;
		while (exit)
		{
			if (texture[texture_index] == 0)	//使われていないテクスチャ情報
			{
				//読み込み処理
				cocos2d::TextureCache* texCache = cocos2d::Director::getInstance()->getTextureCache();
				cocos2d::CCImage::setPNGPremultipliedAlphaEnabled(false);	//ストーレートアルファで読み込む
				cocos2d::Texture2D* tex = texCache->addImage(pszFileName);
				cocos2d::CCImage::setPNGPremultipliedAlphaEnabled(true);	//ステータスを戻しておく


				texture[texture_index] = tex;
				if (!texture[texture_index]) {
					DEBUG_PRINTF("テクスチャの読み込み失敗\n");
				}
				else
				{
					//SpriteStudioで設定されたテクスチャ設定を反映させるための分岐です。
					cocos2d::Texture2D::TexParams texParams;
					switch (wrapmode)
					{
					case SsTexWrapMode::clamp:	//クランプ
						texParams.wrapS = GL_CLAMP_TO_EDGE;
						texParams.wrapT = GL_CLAMP_TO_EDGE;
						break;
					case SsTexWrapMode::repeat:	//リピート
						texParams.wrapS = GL_REPEAT;
						texParams.wrapT = GL_REPEAT;
						break;
					case SsTexWrapMode::mirror:	//ミラー
						texParams.wrapS = GL_MIRRORED_REPEAT;
						texParams.wrapT = GL_MIRRORED_REPEAT;
						break;
					}
					switch (filtermode)
					{
					case SsTexFilterMode::nearlest:	//ニアレストネイバー
						texParams.minFilter = GL_NEAREST;
						texParams.magFilter = GL_NEAREST;
						break;
					case SsTexFilterMode::linear:	//リニア、バイリニア
						texParams.minFilter = GL_LINEAR;
						texParams.magFilter = GL_LINEAR;
						break;
					}
					tex->setTexParameters(texParams);



					isLoad = true;
					rc = texture_index;	//テクスチャハンドルをリソースマネージャに設定する
				}
				exit = false;	//ループ終わり
			}
			else
			{
				if (texture_index == start_index)
				{
					//一周したバッファが開いてない
					DEBUG_PRINTF("テクスチャバッファの空きがない\n");
					exit = false;	//ループ終わり
				}
			}
			//次のインデックスに移動する
			texture_index++;
			if (texture_index >= TEXTURE_MAX)
			{
				texture_index = 0;
			}
		}

		if (isLoad)
		{
		}
		return rc;
	}
	
	/**
	* テクスチャの解放
	*/
	bool SSTextureRelese(long handle)
	{
		/// 解放後も同じ番号で何度も解放処理が呼ばれるので、例外が出ないように作成してください。
		bool rc = true;

		if (texture[handle])
		{
			cocos2d::TextureCache* texCache = cocos2d::Director::getInstance()->getTextureCache();
			texCache->removeTexture(texture[handle]);
			texture[handle] = 0;
		}
		else
		{
			rc = false;
		}

		return rc ;
	}

	/**
	* テクスチャのサイズを取得
	* テクスチャのUVを設定するのに使用します。
	*/
	bool SSGetTextureSize(long handle, int &w, int &h)
	{
		if (texture[handle])
		{
			w = texture[handle]->getPixelsWide();
			h = texture[handle]->getPixelsHigh();
		}
		else
		{
			return false;
		}
		return true;
	}

	//各プレイヤーの描画を行う前の初期化処理
	void SSRenderSetup( void )
	{
#if OPENGLES20
#else
		glDisableClientState(GL_COLOR_ARRAY);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
#endif
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
#if OPENGLES20
#else
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.0);
#endif
		glBlendEquation(GL_FUNC_ADD);
	}

	/**
	パーツカラー用
	ブレンドタイプに応じたテクスチャコンバイナの設定を行う

	ミックスのみコンスタント値を使う。
	他は事前に頂点カラーに対してブレンド率を掛けておく事でαも含めてブレンドに対応している。
	*/
#if OPENGLES20
#else
	void setupPartsColorTextureCombiner(BlendType blendType, VertexFlag colorBlendTarget)
	{
		//static const float oneColor[4] = {1.f,1.f,1.f,1.f};
		float constColor[4] = { 0.5f,0.5f,0.5f,1.0f };
		static const GLuint funcs[] = { GL_INTERPOLATE, GL_MODULATE, GL_ADD, GL_SUBTRACT };
		GLuint func = funcs[(int)blendType];
		GLuint srcRGB = GL_TEXTURE0;
		GLuint dstRGB = GL_PRIMARY_COLOR;

		bool combineAlpha = true;

		switch (blendType)
		{
		case BlendType::BLEND_MIX:
		case BlendType::BLEND_MUL:
		case BlendType::BLEND_ADD:
		case BlendType::BLEND_SUB:
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			// rgb
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, func);

			// mix の場合、特殊
			if (blendType == BlendType::BLEND_MIX)
			{
				if (colorBlendTarget == VertexFlag::VERTEX_FLAG_ONE)
				{
					// 全体なら、const 値で補間する
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_CONSTANT);
					glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constColor);
				}
				else
				{
					// 頂点カラーのアルファをテクスチャに対する頂点カラーの割合にする。
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_PRIMARY_COLOR);

					combineAlpha = false;
				}
				// 強度なので 1 に近付くほど頂点カラーが濃くなるよう SOURCE0 を頂点カラーにしておく。
				std::swap(srcRGB, dstRGB);
			}

			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, srcRGB);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, dstRGB);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
			break;
		case BlendType::BLEND_SCREEN:
		case BlendType::BLEND_EXCLUSION:
		case BlendType::BLEND_INVERT:
		default:
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE0);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_CONSTANT);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			break;
		}

		if (combineAlpha)
		{
			// alpha は常に掛け算
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE0);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
		}
		else
		{
			// ミックス＋頂点単位の場合αブレンドはできない。
			// αはテクスチャを100%使えれば最高だが、そうはいかない。
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE0);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
		}
	}
#endif
	//頂点バッファにパラメータを保存する
	void setClientState(SSV3F_C4B_T2F point, int index, float* uvs, float* colors, float* vertices)
	{
		uvs[0 + (index * 2)] = point.texCoords.u;
		uvs[1 + (index * 2)] = point.texCoords.v;

		colors[0 + (index * 4)] = point.colors.r / 255.0f;
		colors[1 + (index * 4)] = point.colors.g / 255.0f;
		colors[2 + (index * 4)] = point.colors.b / 255.0f;
		colors[3 + (index * 4)] = point.colors.a / 255.0f;

		vertices[0 + (index * 3)] = point.vertices.x;
		vertices[1 + (index * 3)] = point.vertices.y;
		vertices[2 + (index * 3)] = point.vertices.z;
	}

	/**
	* スプライトの表示
	*/
	void SSDrawSprite(CustomSprite *sprite, State *overwrite_state)
	{
		if (sprite->_state.isVisibled == false) return; //非表示なので処理をしない
		if (sprite->_playercontrol == nullptr) return;

		//ステータスから情報を取得し、各プラットフォームに合わせて機能を実装してください。
		State state;
		if (overwrite_state)
		{
			//個別に用意したステートを使用する（エフェクトのパーティクル用）
			state = *overwrite_state;
		}
		else
		{
			state = sprite->_state;
		}
		int tex_index = state.texture.handle;
		if (texture[tex_index] == nullptr)
		{
			return;
		}

		execMask(sprite);	//マスク初期化

		/**
		* OpenGLの3D機能を使用してスプライトを表示します。
		* 下方向がプラスになります。
		* 3Dを使用する場合頂点情報を使用して再現すると頂点変形やUV系のアトリビュートを反映させる事ができます。
		*/
		//描画用頂点情報を作成
		SSV3F_C4B_T2F_Quad quad;
		quad = state.quad;

		//原点補正
		float cx = ((state.rect.size.width) * -(state.pivotX - 0.5f));
		float cy = ((state.rect.size.height) * +(state.pivotY - 0.5f));

		quad.tl.vertices.x += cx;
		quad.tl.vertices.y += cy;
		quad.tr.vertices.x += cx;
		quad.tr.vertices.y += cy;
		quad.bl.vertices.x += cx;
		quad.bl.vertices.y += cy;
		quad.br.vertices.x += cx;
		quad.br.vertices.y += cy;

		float t[16];
		TranslationMatrix(t, quad.tl.vertices.x, quad.tl.vertices.y, 0.0f);

		MultiplyMatrix(t, state.mat, t);
		quad.tl.vertices.x = t[12];
		quad.tl.vertices.y = t[13];
		TranslationMatrix(t, quad.tr.vertices.x, quad.tr.vertices.y, 0.0f);
		MultiplyMatrix(t, state.mat, t);
		quad.tr.vertices.x = t[12];
		quad.tr.vertices.y = t[13];
		TranslationMatrix(t, quad.bl.vertices.x, quad.bl.vertices.y, 0.0f);
		MultiplyMatrix(t, state.mat, t);
		quad.bl.vertices.x = t[12];
		quad.bl.vertices.y = t[13];
		TranslationMatrix(t, quad.br.vertices.x, quad.br.vertices.y, 0.0f);
		MultiplyMatrix(t, state.mat, t);
		quad.br.vertices.x = t[12];
		quad.br.vertices.y = t[13];

		//頂点カラーにアルファを設定
		float alpha = state.Calc_opacity / 255.0f;
		if (state.flags & PART_FLAG_LOCALOPACITY)
		{
			alpha = state.localopacity / 255.0f;	//ローカル不透明度対応
		}

		quad.tl.colors.a = quad.tl.colors.a * alpha;
		quad.tr.colors.a = quad.tr.colors.a * alpha;
		quad.bl.colors.a = quad.bl.colors.a * alpha;
		quad.br.colors.a = quad.br.colors.a * alpha;
		/*
				if (_isContentScaleFactorAuto == true)
				{
					//ContentScaleFactor対応
					float cScale = cocos2d::Director::getInstance()->getContentScaleFactor();
					quad.tl.texCoords.u /= cScale;
					quad.tr.texCoords.u /= cScale;
					quad.bl.texCoords.u /= cScale;
					quad.br.texCoords.u /= cScale;
					quad.tl.texCoords.v /= cScale;
					quad.tr.texCoords.v /= cScale;
					quad.bl.texCoords.v /= cScale;
					quad.br.texCoords.v /= cScale;
				}
		*/
/*
		sprite->_playercontrol->setGLProgram(sprite->_playercontrol->_partColorMULShaderProgram);
		sprite->_playercontrol->getShaderProgram()->use();
		const auto& matrixP = cocos2d::Director::getInstance()->getMatrix(cocos2d::MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
		cocos2d::Mat4 matrixMVP = matrixP;

		glUniformMatrix4fv(sprite->_playercontrol->_uniform_map[WVP], 1, 0, (float *)&matrixMVP.m);
		// テクスチャサンプラ情報をシェーダーに送る  
		glUniform1i(sprite->_playercontrol->_uniform_map[SAMPLER], 0);
*/
		//テクスチャ有効
		int	gl_target = GL_TEXTURE_2D;
		glEnable(gl_target);

		//テクスチャのバインド
		glBindTexture(gl_target, texture[tex_index]->getName());


		//描画モード
		//
		glBlendEquation(GL_FUNC_ADD);
		switch (state.blendfunc)
		{
		case BLEND_MIX:		///< 0 ブレンド（ミックス）
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case BLEND_MUL:		///< 1 乗算
			glBlendFunc(GL_ZERO, GL_SRC_COLOR);
			break;
		case BLEND_ADD:		///< 2 加算
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			break;
		case BLEND_SUB:		///< 3 減算
			glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
#if OPENGLES20
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_DST_ALPHA);
#else
			glBlendFuncSeparateEXT(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_DST_ALPHA);
#endif
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);//とりあえずミックスにしておく
			break;
		case BLEND_MULALPHA:	///< 4 α乗算
			glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case BLEND_SCREEN:		///< 5 スクリーン
			glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);
			break;
		case BLEND_EXCLUSION:	///< 6 除外
			glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
			break;
		case BLEND_INVERT:		///< 7 反転
			glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
			break;

		}

		//メッシュの場合描画
		if (sprite->_partData.type == PARTTYPE_MESH)
		{
			//			this->renderMesh(state->meshPart, alpha);
			return;
		}

#if OPENGLES20
		const auto& matrixP = cocos2d::Director::getInstance()->getMatrix(cocos2d::MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
		cocos2d::Mat4 matrixMVP = matrixP;

		if (state.flags & PART_FLAG_PARTS_COLOR)
		{

			//パーツカラーの反映
			switch ((BlendType)sprite->_state.partsColorFunc)
			{
			case BlendType::BLEND_MIX:
				//シェーダーを適用する
				sprite->_playercontrol->setGLProgram(SSPlayerControl::_partColorMIXShaderProgram);
				sprite->_playercontrol->getShaderProgram()->use();
				//マトリクスを設定
				glUniformMatrix4fv(sprite->_playercontrol->_MIX_uniform_map[WVP], 1, 0, (float *)&matrixMVP.m);
				// テクスチャサンプラ情報をシェーダーに送る  
				glUniform1i(sprite->_playercontrol->_MIX_uniform_map[SAMPLER], 0);
				break;
			case BlendType::BLEND_MUL:
				//シェーダーを適用する
				sprite->_playercontrol->setGLProgram(SSPlayerControl::_partColorMULShaderProgram);
				sprite->_playercontrol->getShaderProgram()->use();
				//マトリクスを設定
				glUniformMatrix4fv(sprite->_playercontrol->_MUL_uniform_map[WVP], 1, 0, (float *)&matrixMVP.m);
				// テクスチャサンプラ情報をシェーダーに送る  
				glUniform1i(sprite->_playercontrol->_MUL_uniform_map[SAMPLER], 0);
				break;
			case BlendType::BLEND_ADD:
				//シェーダーを適用する
				sprite->_playercontrol->setGLProgram(SSPlayerControl::_partColorADDShaderProgram);
				sprite->_playercontrol->getShaderProgram()->use();
				//マトリクスを設定
				glUniformMatrix4fv(sprite->_playercontrol->_ADD_uniform_map[WVP], 1, 0, (float *)&matrixMVP.m);
				// テクスチャサンプラ情報をシェーダーに送る  
				glUniform1i(sprite->_playercontrol->_ADD_uniform_map[SAMPLER], 0);
				break;
			case BlendType::BLEND_SUB:
				//シェーダーを適用する
				sprite->_playercontrol->setGLProgram(SSPlayerControl::_partColorSUBShaderProgram);
				sprite->_playercontrol->getShaderProgram()->use();
				//マトリクスを設定
				glUniformMatrix4fv(sprite->_playercontrol->_SUB_uniform_map[WVP], 1, 0, (float *)&matrixMVP.m);
				// テクスチャサンプラ情報をシェーダーに送る  
				glUniform1i(sprite->_playercontrol->_SUB_uniform_map[SAMPLER], 0);
				break;
			}
		}
		else
		{
			sprite->_playercontrol->setGLProgram(sprite->_playercontrol->_defaultShaderProgram);
			sprite->_playercontrol->getShaderProgram()->use();
		}

/*
		//シェーダーを適用する
		sprite->_playercontrol->setGLProgram(SSPlayerControl::_partColorSUBShaderProgram);
		sprite->_playercontrol->getShaderProgram()->use();
		//マトリクスを設定
		glUniformMatrix4fv(sprite->_playercontrol->_SUB_uniform_map[WVP], 1, 0, (float *)&matrixMVP.m);
		// テクスチャサンプラ情報をシェーダーに送る  
		glUniform1i(sprite->_playercontrol->_SUB_uniform_map[SAMPLER], 0);
*/
#else

		if (state.flags & PART_FLAG_PARTS_COLOR)
		{
			//パーツカラーの反映
			setupPartsColorTextureCombiner((BlendType)sprite->_state.partsColorFunc, (VertexFlag)sprite->_state.partsColorType);
		}
		else
		{
			// カラーは１００％テクスチャ
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE0);
			// αだけ合成
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE0);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
		}

#endif
		cocos2d::GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);

#define kQuadSize sizeof(quad.bl)
		long offset = (long)&quad;

		// vertex
		int diff = offsetof(cocos2d::V3F_C4B_T2F, vertices);
		glVertexAttribPointer(cocos2d::GLProgram::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, kQuadSize, (void*)(offset + diff));

		// texCoods
		diff = offsetof(cocos2d::V3F_C4B_T2F, texCoords);
		glVertexAttribPointer(cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, kQuadSize, (void*)(offset + diff));

		// color
		diff = offsetof(cocos2d::V3F_C4B_T2F, colors);
		glVertexAttribPointer(cocos2d::GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, kQuadSize, (void*)(offset + diff));

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

#define DRAW_DEBUG (1)
#if ( DRAW_DEBUG == 1 )
		// draw bounding box
		{
			cocos2d::Point vertices[4] = {
				cocos2d::Point(quad.tl.vertices.x,quad.tl.vertices.y),
				cocos2d::Point(quad.bl.vertices.x,quad.bl.vertices.y),
				cocos2d::Point(quad.br.vertices.x,quad.br.vertices.y),
				cocos2d::Point(quad.tr.vertices.x,quad.tr.vertices.y),
			};
			ccDrawPoly(vertices, 4, true);
		}
#elif ( DRAW_DEBUG == 2 )
		// draw texture box
		{
			cocos2d::Size s = this->getTextureRect().size;
			cocos2d::Point offsetPix = this->getOffsetPosition();
			cocos2d::Point vertices[4] = {
				cocos2d::Point(offsetPix.x,offsetPix.y), cocos2d::Point(offsetPix.x + s.width,offsetPix.y),
				cocos2d::Point(offsetPix.x + s.width,offsetPix.y + s.height), cocos2d::Point(offsetPix.x,offsetPix.y + s.height)
			};
			ccDrawPoly(vertices, 4, true);
		}
#endif // CC_SPRITE_DEBUG_DRAW


		CC_INCREMENT_GL_DRAWS(1);

		CHECK_GL_ERROR_DEBUG();


/*
		float	uvs[10];			// UVバッファ
		float	colors[4 * 4];		// カラーバッファ
		float	vertices[3 * 5];	// 座標バッファ

		setClientState(quad.tl, 0, uvs, colors, vertices);
		setClientState(quad.tr, 1, uvs, colors, vertices);
		setClientState(quad.bl, 2, uvs, colors, vertices);
		setClientState(quad.br, 3, uvs, colors, vertices);
		
		// UV 配列を指定
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, (GLvoid *)uvs);
		// 頂点カラーの設定
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_FLOAT, 0, (GLvoid *)colors);
		// 頂点バッファの設定
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, (GLvoid *)vertices);

		// 頂点配列を描画
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
*/




//
		glDisable(gl_target);
		glDisable(GL_TEXTURE_2D);
#if OPENGLES20
#else
		glDisable(GL_ALPHA_TEST);
#endif
        //ブレンドモード　減算時の設定を戻す
		glBlendEquation(GL_FUNC_ADD);
#if OPENGLES20
#else
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
#endif
}


	void clearMask()
	{
		glClear(GL_STENCIL_BUFFER_BIT);
		enableMask(false);
	}

	void enableMask(bool flag)
	{

		if (flag)
		{
			glEnable(GL_STENCIL_TEST);
		}
		else {
			glDisable(GL_STENCIL_TEST);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		}
	}

	void execMask(CustomSprite *sprite)
	{
		glEnable(GL_STENCIL_TEST);
		if (sprite->_partData.type == PARTTYPE_MASK)
		{

			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

			if (!(sprite->_maskInfluence)) { //マスクが有効では無い＝重ね合わせる
				glStencilFunc(GL_ALWAYS, 1, ~0);  //常に通過
				glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
				//描画部分を1へ
			}
			else {
				glStencilFunc(GL_ALWAYS, 1, ~0);  //常に通過
				glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
			}
#if OPENGLES20
#else
			glEnable(GL_ALPHA_TEST);
#endif
			//この設定だと
			//1.0fでは必ず抜けないため非表示フラグなし（＝1.0f)のときの挙動は考えたほうがいい

			//不透明度からマスク閾値へ変更
			float mask_alpha = (float)(255 - sprite->_state.masklimen) / 255.0f;
#if OPENGLES20
#else
			glAlphaFunc(GL_GREATER, mask_alpha);
#endif
			sprite->_state.Calc_opacity = 255;	//マスクパーツは不透明度1.0にする
		}
		else {

			if ((sprite->_maskInfluence)) //パーツに対してのマスクが有効か否か
			{
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glStencilFunc(GL_NOTEQUAL, 0x1, 0x1);  //1と等しい
				glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			}
			else {
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glDisable(GL_STENCIL_TEST);
			}

			// 常に無効
#if OPENGLES20
#else
			glDisable(GL_ALPHA_TEST);
#endif
        }

	}

	/**
	* ユーザーデータの取得
	*/
	void SSonUserData(Player *player, UserData *userData)
	{
		//ゲーム側へユーザーデータを設定する関数を呼び出してください。
	}

	/**
	* ユーザーデータの取得
	*/
	void SSPlayEnd(Player *player)
	{
		//ゲーム側へアニメ再生終了を設定する関数を呼び出してください。
	}

	/**
	* 文字コード変換
	*/ 
#if _WIN32
	std::string utf8Togbk(const char *src)
	{
		int len = MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0);
		unsigned short * wszGBK = new unsigned short[len + 1];
		memset(wszGBK, 0, len * 2 + 2);
		MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)src, -1, (LPWSTR)wszGBK, len);

		len = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wszGBK, -1, NULL, 0, NULL, NULL);
		char *szGBK = new char[len + 1];
		memset(szGBK, 0, len + 1);
		WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wszGBK, -1, szGBK, len, NULL, NULL);
		std::string strTemp(szGBK);
		if (strTemp.find('?') != std::string::npos)
		{
			strTemp.assign(src);
		}
		delete[]szGBK;
		delete[]wszGBK;
		return strTemp;
	}
#endif
	/**
	* windows用パスチェック
	*/ 
	bool isAbsolutePath(const std::string& strPath)
	{

#if _WIN32
		std::string strPathAscii = utf8Togbk(strPath.c_str());
#else
        std::string strPathAscii = strPath;
#endif
        if (strPathAscii.length() > 2
			&& ((strPathAscii[0] >= 'a' && strPathAscii[0] <= 'z') || (strPathAscii[0] >= 'A' && strPathAscii[0] <= 'Z'))
			&& strPathAscii[1] == ':')
		{
			return true;
		}
		return false;
	}

};
